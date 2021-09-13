//
// Created by dominic on 08/09/2021.
//

#include "FFMVideo.h"
#include <filesystem>
#include <experimental/filesystem>
#include <syslog.h>

namespace ex = std::experimental;
namespace efs = std::experimental::filesystem;

#define INBUF_SIZE 4096


FFMVideo:: FFMVideo() {
    avcodec_register_all();
}

FFMVideo::~FFMVideo() {
    avformat_close_input(&_pFormatContext);
    av_dict_free(&_avOptions);
    av_dict_free(&_avStreamOptions);
    avcodec_close(_pCodecContext);
    avcodec_free_context(&_pCodecContext);
    av_frame_free(&_pFrame);
    av_packet_free(&_pPacket);
    av_parser_close(_pCodecParserContext);
    avcodec_parameters_free(&_pCodecParams);
}

bool FFMVideo::Initialize() {
    av_dict_set(&_avOptions, "b", "2.5M", 0);
    _pFormatContext = avformat_alloc_context();
    _pCodecParams = avcodec_parameters_alloc();

    _pFrame = av_frame_alloc();
    _pPacket = av_packet_alloc();

    _pCodec = avcodec_find_decoder(AV_CODEC_ID_MPEG4 );
    _pCodecParserContext = av_parser_init(_pCodec->id);
    _pCodecContext = avcodec_alloc_context3(_pCodec);
    _pInputFormat = av_find_input_format("ic");

    if(avcodec_open2(_pCodecContext, _pCodec, nullptr) >= 0) {
        syslog(LOG_INFO, "Successfully opened codec context");
        return true;
    } else {
        syslog(LOG_ERR, "Error opening coded context");
    }
    return false;
}

bool FFMVideo::LoadFile(const std::string &inputFile) {
    auto rtspProtocolTag = inputFile.find("rtsp://");
    if(rtspProtocolTag != std::string::npos) {
        return LoadStream(inputFile);
    } else {
        return LoadMediaFile(inputFile);
    }
}

bool FFMVideo::LoadMediaFile(const std::string &inputFile) {
    if(!inputFile.empty()) {
        syslog(LOG_INFO, "Attempting to load file: %s", inputFile.c_str());

        if(efs::exists(inputFile.c_str())) {
            if(avformat_open_input(&_pFormatContext, inputFile.c_str(), _pInputFormat, &_avOptions) == 0) {

                auto result = LoadMedia(inputFile);

                syslog(LOG_INFO, "Successfully loaded file");
                return result;
            } else {
                syslog(LOG_ERR, "Error opening file: avformat_open_input failed");
            }
        } else {
            syslog(LOG_ERR, "Error opening file: File does not exist");
        }
    }
    return false;
}

bool FFMVideo::LoadStream(const std::string &path) {
    if(!path.empty()) {
        syslog(LOG_INFO, "Attempting to load stream: %s", path.c_str());

        if(avformat_open_input(&_pFormatContext, path.c_str(), _pInputFormat, &_avOptions) == 0) {
            // Note: I get a seg fault if I call avformat_find_stream_info with _avOptions
            if(avformat_find_stream_info(_pFormatContext, nullptr) >= 0) {

                auto result = LoadMedia(path);

                syslog(LOG_INFO, "Successfully loaded stream & info");
                return result;
            } else {
                syslog(LOG_ERR, "Error getting video info: avformat_find_stream_info failed");
            }
        } else {
            syslog(LOG_ERR, "Error opening file: avformat_open_input failed");
        }
    }
    return false;
}

static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize, char * filename)
{
    FILE *f;
    int i;

    f = fopen(filename,"wb");
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}

static void decode(AVCodecContext * dec_ctx, AVFrame * frame, AVPacket * pkt, const std::string& filename)
{
    char buf[1024];
    int ret;

    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        syslog(LOG_ERR, "Error sending a packet for decoding");
        return;
    }

    while (ret >= 0) {
        syslog(LOG_DEBUG, "Receiving Frame");
        ret = avcodec_receive_frame(dec_ctx, frame);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return;
        }
        else if (ret < 0) {
            syslog(LOG_ERR, "Error during decoding");
        }

        printf("saving frame %3d\n", dec_ctx->frame_number);
        fflush(stdout);

        snprintf(buf, sizeof(buf), "%s-%d", filename.c_str(), dec_ctx->frame_number);
        pgm_save(frame->data[0], frame->linesize[0], frame->width, frame->height, buf);
    }
}

bool FFMVideo::LoadMedia(const std::string& inputFile) {
    bool result = false;
    FILE * file = fopen(inputFile.c_str(), "rb");

    if(file) {
        int ret = 0;
        size_t data_size;
        uint8_t * data;
        uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];

        while (!feof(file)) {
            /* read raw data from the input file */
            data_size = fread(inbuf, 1, INBUF_SIZE, file);
            if (data_size) {

                /* use the parser to split the data into frames */
                data = inbuf;
                while (data_size > 0) {
                    ret = av_parser_parse2(_pCodecParserContext, _pCodecContext, &_pPacket->data, &_pPacket->size,
                                           data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
                    if (ret >= 0) {
                        data += ret;
                        data_size -= ret;

                        if (_pPacket->size) {
                            decode(_pCodecContext, _pFrame, _pPacket, "testing-output.mp4");
                            result = true;
                        }
                    } else {
                        syslog(LOG_ERR, "Error while parsing");
                    }
                }
            }
        }
        fclose(file);
    } else {
        syslog(LOG_ERR, "Unable to open file: %s", inputFile.c_str());
    }

    return result;
}
