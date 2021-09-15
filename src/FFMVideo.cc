//
// Created by dominic on 08/09/2021.
//

#include "FFMVideo.h"
#include <filesystem>
#include <syslog.h>

#include "FFMUtils.h"

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

    for(auto frame : _videoFrames)
        if(frame != nullptr)
            av_frame_free(&frame);

    for(auto frame : _audioFrames)
        if(frame != nullptr)
            av_frame_free(&frame);

    for(auto frame : _subtitleFrames)
        if(frame)
            av_frame_free(&frame);

    _videoFrames.clear();
    _audioFrames.clear();
    _subtitleFrames.clear();
}

bool FFMVideo::Initialize() {
    av_dict_set(&_avOptions, "b", "2.5M", 0);
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

                    auto packet = av_packet_alloc();
                    while(av_read_frame(_pFormatContext, packet) >= 0) {
                        auto latestVideo = av_frame_alloc();
                        auto latestAudio = av_frame_alloc();

                        if (packet->stream_index == videoStreamIndex) {
                            ret = utils::decode_packet(_pVideoCodecContext, packet, &latestVideo);
                            if(ret >= 0) {
                                if(ret > 0) {
                                    _videoFrames.push_back(latestVideo);
                                } else {
                                    av_frame_free(&latestVideo);
                                }
                            } else {
                                syslog(LOG_DEBUG, "Unable to decode packet");
                                av_frame_free(&latestVideo);
                            }
                        }
                        else if (packet->stream_index == audioStreamIndex) {
                            ret = utils::decode_packet(_pAudioCodecContext, packet, &latestAudio);
                            if(ret >= 0) {
                                if(ret > 0) {
                                    _audioFrames.push_back(latestAudio);
                                } else {
                                    av_frame_free(&latestAudio);
                                }
                            } else {
                                syslog(LOG_DEBUG, "Unable to decode packet");
                                av_frame_free(&latestAudio);
                            }
                        }

                        av_packet_unref(packet);
                        if(ret < 0)
                            break;
                    }
                    av_packet_free(&packet);

                    syslog(LOG_DEBUG, "Video frame stack has %zu entries", _videoFrames.size());
                    syslog(LOG_DEBUG, "Audio frame stack has %zu entries", _audioFrames.size());
                    syslog(LOG_DEBUG, "Subtitle frame stack has %zu entries", _subtitleFrames.size());
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
