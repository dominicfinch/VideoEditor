
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

    int decode_packet(AVCodecContext *dec, const AVPacket *pkt, AVFrame ** frame);

    int draw_grayscale_image(const std::string& filepath, AVFrame * frame);
}


#endif //VIDEO_EDITOR_FFMUTILS_H
