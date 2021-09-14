
#ifndef VIDEO_EDITOR_FFMUTILS_H
#define VIDEO_EDITOR_FFMUTILS_H

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/frame.h>
    #include <libavutil/imgutils.h>
};
#include <string>

namespace utils {
    int open_codec_context(AVCodecContext ** dec_ctx, AVFormatContext * fmt_ctx, enum AVMediaType type);

    int decode_packet(const std::string& filepath, AVCodecContext *dec, const AVPacket *pkt, AVFrame * frame);

    int output_video_frame(const std::string& filepath, AVFrame * frame, enum AVPixelFormat pix_fmt, int height = 800, int width = 600);

    //int output_audio_frame(const std::string& filepath, AVFrame * frame, int height, int width);
}


#endif //VIDEO_EDITOR_FFMUTILS_H
