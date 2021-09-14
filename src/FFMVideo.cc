//
// Created by dominic on 08/09/2021.
//

#include "FFMVideo.h"
#include <filesystem>
#include <syslog.h>

#include "utils/FFMUtils.h"

namespace fs = std::filesystem;


FFMVideo:: FFMVideo() {
    //avcodec_register_all();
}

FFMVideo::~FFMVideo() {
    avformat_close_input(&_pFormatContext);
    av_dict_free(&_avOptions);

    av_free(_pVideoStream);
    av_free(_pAudioCodecContext);
    av_free(_pSubtitleCodecContext);

    avcodec_close(_pVideoCodecContext);
    avcodec_free_context(&_pVideoCodecContext);

    avcodec_close(_pAudioCodecContext);
    avcodec_free_context(&_pAudioCodecContext);

    av_frame_free(&_pFrame);
    av_packet_free(&_pPacket);

    avcodec_close(_pSubtitleCodecContext);
    avcodec_free_context(&_pSubtitleCodecContext);
}

bool FFMVideo::Initialize() {
    av_dict_set(&_avOptions, "b", "2.5M", 0);

    _pFrame = av_frame_alloc();
    _pPacket = av_packet_alloc();

    return true;
}

bool FFMVideo::LoadMedia(const std::string& inputFile) {
    if(!inputFile.empty()) {
        syslog(LOG_INFO, "Attempting to load input '%s'", inputFile.c_str());

        int ret = 0;

        if(fs::exists(inputFile.c_str())) {
            if(avformat_open_input(&_pFormatContext, inputFile.c_str(), _pInputFormat, &_avOptions) == 0) {
                if(avformat_find_stream_info(_pFormatContext, nullptr) >= 0) {
                    auto videoStreamIndex = utils::open_codec_context(_pVideoCodecContext, _pFormatContext, AVMediaType::AVMEDIA_TYPE_VIDEO);
                    auto audioStreamIndex = utils::open_codec_context(_pAudioCodecContext, _pFormatContext, AVMediaType::AVMEDIA_TYPE_AUDIO);
                    auto subtitleStreamIndex = utils::open_codec_context(_pSubtitleCodecContext, _pFormatContext, AVMediaType::AVMEDIA_TYPE_SUBTITLE);

                    if(videoStreamIndex >= 0) {
                        _pVideoStream = _pFormatContext->streams[videoStreamIndex];
                    }

                    if(audioStreamIndex >= 0) {
                        _pAudioStream = _pFormatContext->streams[audioStreamIndex];
                    }

                    if(subtitleStreamIndex >=0) {
                        _pSubtitleStream = _pFormatContext->streams[subtitleStreamIndex];
                    }

                    while(av_read_frame(_pFormatContext, _pPacket) >= 0) {
                        // check if the packet belongs to a stream we are interested in, otherwise
                        // skip it
                        if (_pPacket->stream_index == videoStreamIndex)
                            ret = utils::decode_packet(_pVideoCodecContext, _pPacket, _pFrame);
                        else if (_pPacket->stream_index == audioStreamIndex)
                            ret = utils::decode_packet(_pAudioCodecContext, _pPacket, _pFrame);
                        av_packet_unref(_pPacket);
                        if(ret < 0)
                            break;
                    }

                    if (_pVideoStream)
                        utils::decode_packet(_pVideoCodecContext, nullptr, nullptr);
                    if (_pAudioStream)
                        utils::decode_packet(_pAudioCodecContext, nullptr, nullptr);

                    syslog(LOG_INFO, "Successfully loaded stream & info");
                    return true;
                } else {
                    syslog(LOG_ERR, "Error getting video info: avformat_find_stream_info failed");
                }
            } else {
                syslog(LOG_ERR, "Error opening file: avformat_open_input failed");
            }
        } else {
            syslog(LOG_ERR, "Error opening file: File does not exist");
        }
    }
    return false;
}