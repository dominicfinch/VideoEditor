//
// Created by dominic on 16/09/2021.
//

#include "FFMVideoImpl.h"
#include "utils/macros.h"
#include <filesystem>
#include <syslog.h>

namespace fs = std::filesystem;

namespace vsg {

    namespace internal {
        int open_codec_context(AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type) {
            int stream_index = -1;
            AVStream *st;
            AVCodec *dec = nullptr;

            if (fmt_ctx) {
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
    }

    FFMVideoImpl::FFMVideoImpl() {

    }

    FFMVideoImpl::~FFMVideoImpl() {
        Shutdown();
    }

    int FFMVideoImpl::Initialize(AVDictionary * settings) {
        if(!_initialized) {
            if(settings) {
                _avOptions = settings;
            } else {
                av_dict_set(&_avOptions, "b", "2.5M", 0);
            }
            _initialized = true;
        } else {
            syslog(LOG_WARNING, "Attempting to initialize class instance multiple times (%s)", __CLASS_NAME_CSTR__);
            return -1;
        }
        return 0;
    }

    int FFMVideoImpl::LoadSource(const std::string &source) {
        int ret = 0;
        if(_initialized && !source.empty()) {
            syslog(LOG_INFO, "Attempting to load input '%s'", source.c_str());
            if(avformat_open_input(&_pFormatContext, source.c_str(), _pInputFormat, &_avOptions) == 0) {
                if (avformat_find_stream_info(_pFormatContext, nullptr) >= 0) {
                    auto videoStreamIndex = internal::open_codec_context(&_pVideoCodecContext, _pFormatContext,
                                                                      AVMediaType::AVMEDIA_TYPE_VIDEO);
                    auto audioStreamIndex = internal::open_codec_context(&_pAudioCodecContext, _pFormatContext,
                                                                      AVMediaType::AVMEDIA_TYPE_AUDIO);
                    auto subtitleStreamIndex = internal::open_codec_context(&_pSubtitleCodecContext, _pFormatContext,
                                                                         AVMediaType::AVMEDIA_TYPE_SUBTITLE);

                    if (videoStreamIndex >= 0) {
                        _pVideoStream = _pFormatContext->streams[videoStreamIndex];
                        _streamIndexes[0] = videoStreamIndex;
                    }

                    if (audioStreamIndex >= 0) {
                        _pAudioStream = _pFormatContext->streams[audioStreamIndex];
                        _streamIndexes[1] = audioStreamIndex;
                    }

                    if (subtitleStreamIndex >= 0) {
                        _pSubtitleStream = _pFormatContext->streams[subtitleStreamIndex];
                        _streamIndexes[2] = subtitleStreamIndex;
                    }

                    _source = source;
                    ret = 0;
                    syslog(LOG_INFO, "Successfully opened input source");
                } else {
                    syslog(LOG_ERR, "Unable to load stream data from input source");
                    ret = -3;
                }
            } else {
                syslog(LOG_ERR, "Unable to open input context");
                ret = -2;
            }
        } else {
            syslog(LOG_ERR, "Error loading source. Incorrectly initialized or invalid source");
            ret = -1;
        }
        return ret;
    }

    int FFMVideoImpl::Shutdown() {
        avformat_close_input(&_pFormatContext);
        avformat_free_context(_pFormatContext);
        av_dict_free(&_avOptions);

        avcodec_close(_pVideoCodecContext);
        avcodec_free_context(&_pVideoCodecContext);

        avcodec_close(_pAudioCodecContext);
        avcodec_free_context(&_pAudioCodecContext);

        avcodec_close(_pSubtitleCodecContext);
        avcodec_free_context(&_pSubtitleCodecContext);

        if(_lastPacket)
            av_packet_free(&_lastPacket);

        if(_lastFrame)
            av_frame_free(&_lastFrame);

        for(auto frame : _frameBuffer) {
            if (frame) {
                av_frame_free(&frame);
            }
        }
        _frameBuffer.size();
        return 0;
    }

    int FFMVideoImpl::SendPacket() {
        if(_initialized) {
            if (!_lastPacket)
                _lastPacket = av_packet_alloc();

            if (av_read_frame(_pFormatContext, _lastPacket) == 0) {
                if(!_lastFrame)
                    _lastFrame = av_frame_alloc();

                /*
                if(_lastPacket->stream_index == _streamIndexes[0]) {

                } else if(_lastPacket->stream_index == _streamIndexes[1]) {

                } else if(_lastPacket->stream_index == _streamIndexes[2]) {

                }
                 */
            } else {
                syslog(LOG_ERR, "Error sending packet");
            }
            av_packet_unref(_lastPacket);
            return 0;
        } else {
            return -1;
        }
    }

    int FFMVideoImpl::ReceivePacket() {
        if(!_lastFrame)
            _lastFrame = av_frame_alloc();

        if(_lastPacket && _lastPacket->stream_index == _streamIndexes[0]) {
            // Video Packets
            auto ret = decode_packet(_pVideoCodecContext, _lastPacket, &_lastFrame);
            if(ret >= 0) {
                // Successfully decoded frame
                if(ret > 0) {
                    if(_frameBuffer.size() >= _maxFrameBufferSize) {
                        auto numberToRemove = _maxFrameBufferSize - _frameBuffer.size();
                        for(auto i=0; i<numberToRemove; i++) {
                            av_frame_free(&_frameBuffer[i]);
                            _frameBuffer.pop_front();
                        }
                    }
                    _frameBuffer.push_back(_lastFrame);
                } else {
                    // Empty Frame
                }
            } else {
                syslog(LOG_DEBUG, "Unable to decode packet");
            }
            av_frame_free(&_lastFrame);
        } else if(_lastPacket && _lastPacket->stream_index == _streamIndexes[1]) {
            // Audio Frames
        } else if(_lastPacket && _lastPacket->stream_index == _streamIndexes[2]) {
            // Subtitle Frames
        }

        return 0;
    }


    int FFMVideoImpl::decode_packet(AVCodecContext * dec, const AVPacket * pkt, AVFrame ** frame) {
        int ret = 0;
        char err[AV_ERROR_MAX_STRING_SIZE];

        if(dec && pkt && frame) {
            // submit the packet to the decoder
            ret = avcodec_send_packet(dec, pkt);
            if (ret < 0) {
                av_make_error_string(err, AV_ERROR_MAX_STRING_SIZE, ret);
                syslog(LOG_ERR, "Error submitting a packet for decoding (%s)\n", err);
                return ret;
            }

            // get all the available frames from the decoder
            while(ret >= 0) {
                ret = avcodec_receive_frame(dec, *frame);
                if(ret >= 0) {
                    if(dec->codec->type == AVMEDIA_TYPE_VIDEO) {
                        return (*frame)->width * (*frame)->height;
                    } else if(dec->codec->type == AVMEDIA_TYPE_AUDIO) {
                        return (*frame)->nb_samples;
                    }

                    if(ret < 0)
                        return ret;
                } else {
                    if(ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
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
}