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
    avformat_free_context(_pFormatContext);
    av_dict_free(&_avOptions);

    avcodec_close(_pVideoCodecContext);
    avcodec_free_context(&_pVideoCodecContext);

    avcodec_close(_pAudioCodecContext);
    avcodec_free_context(&_pAudioCodecContext);

    avcodec_close(_pSubtitleCodecContext);
    avcodec_free_context(&_pSubtitleCodecContext);
    av_packet_free(&_pPacket);

    for(auto frame : _videoFrames)
        av_frame_free(&frame);

    for(auto frame : _audioFrames)
        av_frame_free(&frame);

    for(auto frame : _subtitleFrames)
        av_frame_free(&frame);

    _videoFrames.clear();
    _audioFrames.clear();
    _subtitleFrames.clear();
}

bool FFMVideo::Initialize() {
    av_dict_set(&_avOptions, "b", "2.5M", 0);
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
                    auto videoStreamIndex = utils::open_codec_context(&_pVideoCodecContext, _pFormatContext, AVMediaType::AVMEDIA_TYPE_VIDEO);
                    auto audioStreamIndex = utils::open_codec_context(&_pAudioCodecContext, _pFormatContext, AVMediaType::AVMEDIA_TYPE_AUDIO);
                    auto subtitleStreamIndex = utils::open_codec_context(&_pSubtitleCodecContext, _pFormatContext, AVMediaType::AVMEDIA_TYPE_SUBTITLE);

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
                        if (_pPacket->stream_index == videoStreamIndex) {
                            _videoFrames.push_back(av_frame_alloc());
                            ret = utils::decode_packet(_pVideoCodecContext, _pPacket, _videoFrames.back());
                        }
                        else if (_pPacket->stream_index == audioStreamIndex) {
                            _audioFrames.push_back(av_frame_alloc());
                            ret = utils::decode_packet(_pAudioCodecContext, _pPacket, _audioFrames.back());
                        }
                        else if(_pPacket->stream_index == subtitleStreamIndex) {
                            _subtitleFrames.push_back(av_frame_alloc());
                            ret = utils::decode_packet(_pSubtitleCodecContext, _pPacket, _subtitleFrames.back());
                        }
                        av_packet_unref(_pPacket);
                        if(ret < 0)
                            break;
                    }

                    syslog(LOG_DEBUG, "Video frame stack has %zu entries", _videoFrames.size());
                    syslog(LOG_DEBUG, "Audio frame stack has %zu entries", _audioFrames.size());
                    syslog(LOG_DEBUG, "Subtitle frame stack has %zu entries", _subtitleFrames.size());
                    syslog(LOG_INFO, "Successfully loaded stream & info");

                    // Try saving the first video frame as an image //
                    if(_videoFrames.size() > 0)
                        utils::output_video_frame("output.bmp", _videoFrames[0], AV_PIX_FMT_RGB0);
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
