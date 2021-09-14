//
// Created by dominic on 08/09/2021.
//

#ifndef VIDEO_MANIPULATION_FFMVIDEO_H
#define VIDEO_MANIPULATION_FFMVIDEO_H

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavformat/avio.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/file.h>
    #include <libavutil/dict.h>
};
#include <string>
#include <vector>


class FFMVideo {
public:
    FFMVideo();
    virtual ~FFMVideo();

    bool Initialize();
    bool LoadMedia(const std::string& inputFile);

private:
    AVFormatContext * _pFormatContext = nullptr;
    AVInputFormat * _pInputFormat = nullptr;

    AVStream * _pVideoStream = nullptr;
    AVStream * _pAudioStream = nullptr;
    AVStream * _pSubtitleStream = nullptr;

    AVCodecContext * _pVideoCodecContext = nullptr;
    AVCodecContext * _pAudioCodecContext = nullptr;
    AVCodecContext * _pSubtitleCodecContext = nullptr;

    AVPacket * _pPacket = nullptr;
    AVFrame * _pFrame = nullptr;

    AVDictionary * _avOptions = nullptr;
};


#endif //VIDEO_MANIPULATION_FFMVIDEO_H
