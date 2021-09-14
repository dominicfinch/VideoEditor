
#include "utils/FFMUtils.h"
#include <syslog.h>

namespace utils {
    int open_codec_context(AVCodecContext * dec_ctx, AVFormatContext * fmt_ctx, enum AVMediaType type) {
        int stream_index = -1;
        AVStream *st;
        AVCodec * dec = nullptr;

        if(fmt_ctx) {
            stream_index = av_find_best_stream(fmt_ctx, type, -1, -1, &dec, 0);
            if (stream_index >= 0) {
                st = fmt_ctx->streams[stream_index];

                /* find decoder for the stream */
                /*
                dec = avcodec_find_decoder(st->codecpar->codec_id);
                if(dec) {

                } else {
                    syslog(LOG_ERR, "Unable to find decoder");
                    return AVERROR(EINVAL);
                }
                 */

                /* Allocate a codec context for the decoder */
                dec_ctx = avcodec_alloc_context3(dec);
                if (!dec_ctx) {
                    syslog(LOG_ERR, "Failed to allocate the %s codec context", av_get_media_type_string(type));
                    return AVERROR(EINVAL);
                }

                /* Copy codec parameters from input stream to output codec context */
                if (avcodec_parameters_to_context(dec_ctx, st->codecpar) < 0) {
                    syslog(LOG_ERR, "Failed to copy %s codec parameters to decoder context",
                           av_get_media_type_string(type));
                    return AVERROR(EINVAL);
                }

                /* Init the decoders */
                if (avcodec_open2(dec_ctx, dec, nullptr) < 0) {
                    syslog(LOG_ERR, "Failed to open codec: %s", av_get_media_type_string(type));
                    return AVERROR(EINVAL);
                }
            } else {
                syslog(LOG_ERR, "Unable to find stream");
            }
        }
        return stream_index;
    }

    int decode_packet(AVCodecContext *dec, const AVPacket *pkt, AVFrame * frame)
    {
        int ret = 0;

        // submit the packet to the decoder
        ret = avcodec_send_packet(dec, pkt);
        if (ret < 0) {
            //fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
            return ret;
        }

        // get all the available frames from the decoder
        while (ret >= 0) {
            ret = avcodec_receive_frame(dec, frame);
            if (ret < 0) {
                // those two return values are special and mean there is no output
                // frame available, but there were no errors during decoding
                if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                    return 0;

                //fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
                return ret;
            }

            // write the frame data to output file
            /*
            if (dec->codec->type == AVMEDIA_TYPE_VIDEO)
                ret = output_video_frame(frame);
            else
                ret = output_audio_frame(frame);
            */

            av_frame_unref(frame);
            if (ret < 0)
                return ret;
        }

        return 0;
    }

}
