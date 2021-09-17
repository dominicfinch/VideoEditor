//
// Created by dominic on 16/09/2021.
//

#ifndef VIDEO_EDITOR_FFMVIDEOIMPL_H
#define VIDEO_EDITOR_FFMVIDEOIMPL_H

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavformat/avio.h>
    #include <libavcodec/avcodec.h>
    #include <libavutil/file.h>
    #include <libavutil/dict.h>
    #include <libavutil/imgutils.h>
};
#include <string>
#include <deque>
#include <memory>
#include "interfaces/ivideo.h"

namespace vsg {

    namespace internal {
        int open_codec_context(AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
    }

    class FFMVideoImpl {
    public:
        FFMVideoImpl();
        ~FFMVideoImpl();

        int Initialize(AVDictionary * settings = nullptr);
        int LoadSource(const std::string& source);

        int SendPacket();
        int ReceivePacket();
        //int SetCurrentFrame(int pts);

        AVStream * VideoStream() { return _pVideoStream; }
        AVStream * AudioStream() { return _pAudioStream; }

        AVFormatContext * FormatContext() { return _pFormatContext; }
        AVInputFormat * InputFormat() { return _pInputFormat; }



    protected:
        int Shutdown();
        int decode_packet(AVCodecContext *dec, const AVPacket *pkt, AVFrame ** frame);

    private:

        std::string _source;
        bool _initialized = false;
        AVFormatContext * _pFormatContext = nullptr;
        AVInputFormat * _pInputFormat = nullptr;

        AVStream * _pVideoStream = nullptr;
        AVStream * _pAudioStream = nullptr;
        AVStream * _pSubtitleStream = nullptr;
        int _streamIndexes[3] = {-1, -1, -1};
        AVPacket * _lastPacket = nullptr;
        AVFrame * _lastFrame = nullptr;

        const int _maxFrameBufferSize = 30*60*2;        // 3600 (Assuming: 30 FPS, maximum clip length: 60 seconds and 2 channels - video & audio)

        std::deque<AVPacket*> _packetQueue;
        std::deque<AVFrame*> _frameBuffer;

        AVCodecContext * _pVideoCodecContext = nullptr;
        AVCodecContext * _pAudioCodecContext = nullptr;
        AVCodecContext * _pSubtitleCodecContext = nullptr;
        AVDictionary * _avOptions = nullptr;
    };
}

#endif //VIDEO_EDITOR_FFMVIDEOIMPL_H
