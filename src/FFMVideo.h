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
#include "interfaces/video.h"


class FFMVideo: public IVideo {
public:
    FFMVideo();
    virtual ~FFMVideo();

    bool Initialize();
    bool LoadFile(const std::string& inputFile);


protected:
    bool LoadMediaFile(const std::string& inputFile);
    bool LoadStream(const std::string& path);

    bool LoadMedia(const std::string& inputFile);

private:
    AVFormatContext * _pFormatContext = nullptr;
    AVInputFormat * _pInputFormat = nullptr;
    AVCodec * _pCodec = nullptr;
    AVFrame * _pFrame = nullptr;
    AVPacket * _pPacket = nullptr;

    AVCodecParserContext * _pCodecParserContext = nullptr;
    AVCodecContext * _pCodecContext = nullptr;
    AVCodecParameters * _pCodecParams = nullptr;

    AVDictionary * _avOptions = nullptr;
    AVDictionary * _avStreamOptions = nullptr;

    std::string _filepath;
};


#endif //VIDEO_MANIPULATION_FFMVIDEO_H
