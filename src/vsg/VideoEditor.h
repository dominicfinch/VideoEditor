//
// Created by dominic on 16/09/2021.
//

#ifndef VIDEO_EDITOR_VIDEOEDITOR_H
#define VIDEO_EDITOR_VIDEOEDITOR_H

#include "ffmpeg/FFMVideo.h"
#include "gstreamer/GSVideo.h"
#include <QtCore/QCommandLineParser>
#include <thread>

namespace vsg {

    enum class SupportedVideoLibraries {
        FFMpeg, GStreamer
    };

    enum class SupportedLogLevels {
        debug, info, warn, error
    };

    class VideoEditorParams {
    public:
        QString inputFilepath;
        QString outputFilepath;
        SupportedVideoLibraries library = SupportedVideoLibraries::FFMpeg;
        SupportedLogLevels logLevel = SupportedLogLevels::info;
    };

    class VideoEditor {
    public:
        VideoEditor();
        ~VideoEditor();

        bool Initialize(int argc, char * argv[]);
        const VideoEditorParams& Configuration() { return _params; }
        int Exec();

    private:
        vsg::FFMVideo * _ffmVideo = nullptr;
        vsg::GSVideo * _gsVideo = nullptr;
        int processArguments(int argc, char * argv[]);

        int _argc;
        char ** _argv;

        QCommandLineParser _parser;
        VideoEditorParams _params;
        bool _initialized = false;

        std::thread _frameReaderThread;
        std::thread _frameDecoderThread;
        bool _reading = false, _decoding = false;

        std::function<void()> _decodingLambda;
        std::function<void()> _readingLambda;

        //std::shared_ptr<EventListener> _eventListener = nullptr;
        //std::shared_ptr<EventEmitter> _eventEmitter = nullptr;
    };
}

#endif //VIDEO_EDITOR_VIDEOEDITOR_H
