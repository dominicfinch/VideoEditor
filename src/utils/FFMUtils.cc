
#include "utils/FFMUtils.h"
#include <syslog.h>

namespace utils {
    int open_codec_context(AVCodecContext ** dec_ctx, AVFormatContext * fmt_ctx, enum AVMediaType type) {
        int stream_index = -1;
        AVStream *st;
        AVCodec * dec = nullptr;

        if(fmt_ctx) {
            stream_index = av_find_best_stream(fmt_ctx, type, -1, -1, &dec, 0);
            if (stream_index >= 0) {
                st = fmt_ctx->streams[stream_index];
                /* Allocate a codec context for the decoder */
                *dec_ctx = avcodec_alloc_context3(dec);
                if (*dec_ctx == nullptr) {
                    syslog(LOG_ERR, "Failed to allocate the %s codec context", av_get_media_type_string(type));
                    return AVERROR(EINVAL);
                }

                /* Copy codec parameters from input stream to output codec context */
                if (avcodec_parameters_to_context(*dec_ctx, st->codecpar) < 0) {
                    syslog(LOG_ERR, "Failed to copy %s codec parameters to decoder context",
                           av_get_media_type_string(type));
                    return AVERROR(EINVAL);
                }

                /* Init the decoders */
                if (avcodec_open2(*dec_ctx, dec, nullptr) < 0) {
                    syslog(LOG_ERR, "Failed to open codec: %s", av_get_media_type_string(type));
                    return AVERROR(EINVAL);
                }
            } else {
                syslog(LOG_ERR, "Unable to find stream of type: %s", av_get_media_type_string(type));
            }
        }
        return stream_index;
    }

    int decode_packet(AVCodecContext * dec, const AVPacket * pkt, AVFrame * frame) {
        int ret = 0, decodedPacketCount = 0;
        char err[AV_ERROR_MAX_STRING_SIZE];

        if(pkt && frame) {
            // submit the packet to the decoder
            ret = avcodec_send_packet(dec, pkt);
            if (ret < 0) {
                av_make_error_string(err, AV_ERROR_MAX_STRING_SIZE, ret);
                syslog(LOG_ERR, "Error submitting a packet for decoding (%s)\n", err);
                return ret;
            }

            // get all the available frames from the decoder
            while (ret >= 0) {
                ret = avcodec_receive_frame(dec, frame);
                if (ret < 0) {
                    if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                        return 0;

                    av_make_error_string(err, AV_ERROR_MAX_STRING_SIZE, ret);
                    syslog(LOG_ERR, "Error getting frames from the decoder: %s", err);
                    return ret;
                }
            }
            return 0;
        } else {
            return -1;
        }
    }

    int output_video_frame(const std::string& filepath, AVFrame * frame, enum AVPixelFormat pix_fmt, int height, int width) {
        if(frame) {
            FILE * output_file = fopen(filepath.c_str(), "wb");
            uint8_t * video_dst_data[4] = { nullptr };
            int video_dst_linesize[4] = { 0 };
            int video_dst_bufsize = av_image_alloc(video_dst_data, video_dst_linesize, width, height, pix_fmt, 1);

            if(video_dst_bufsize > 0) {
                av_image_copy(video_dst_data, video_dst_linesize, (const uint8_t **) (frame->data), frame->linesize, pix_fmt, width, height);
                fwrite(video_dst_data[0], 1, video_dst_bufsize, output_file);
            }

            if(output_file)
                fclose(output_file);
        } else {
            return -1;
        }
        return 0;
    }

    /*
    int output_audio_frame(AVFrame *frame)
    {
        size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample(frame->format);
        fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);
        return 0;
    }
    */
}
