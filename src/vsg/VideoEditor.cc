//
// Created by dominic on 16/09/2021.
//

#include "VideoEditor.h"
#include <syslog.h>

vsg::VideoEditor::VideoEditor() {
    //_parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);
    _parser.setApplicationDescription("VideoSoft Video Editor");
    _parser.addOption(QCommandLineOption("input", "Input File", QCoreApplication::translate("main", "filepath"),
                                         QCoreApplication::translate("main", "")));
    _parser.addOption(QCommandLineOption("output", "Output File", QCoreApplication::translate("main", "filepath"),
                                         QCoreApplication::translate("main", "output")));
    _parser.addOption(QCommandLineOption("library", "Library to use", QCoreApplication::translate("main", "ffmpeg|gstreamer"),
                                         QCoreApplication::translate("main", "ffmpeg")));
    _parser.addOption(QCommandLineOption("loglevel", "Log Level", QCoreApplication::translate("main", "debug|info|warn|error"),
                                         QCoreApplication::translate("main", "info")));
    _parser.addHelpOption();
    _parser.addVersionOption();
}

vsg::VideoEditor::~VideoEditor() {
    syslog(LOG_INFO, "Shutting down Video Editor");

    _reading = false;
    _decoding = false;

    //_frameSenderThread.~thread();
    //_frameReceiverThread.~thread();


    if(_ffmVideo)
        delete _ffmVideo;

    if(_gsVideo)
        delete _gsVideo;
}

bool vsg::VideoEditor::Initialize(int argc, char * argv[]) {
    auto errorCount = processArguments(argc, argv);
    if(errorCount == 0) {
        _initialized = true;
        return _initialized;
    } else
        return false;
}

int vsg::VideoEditor::processArguments(int argc, char * argv[]) {
    QStringList args;
    for(auto i=0; i<argc; i++)
        args.push_back(argv[i]);

    int errorCount = (_parser.parse(args) ? 0 : 1);
    if(errorCount == 0) {
        if(_parser.isSet("help")) {
            _parser.showHelp();
        } else if(_parser.isSet("version")) {
            _parser.showVersion();
        } else {
            auto inputPath = _parser.value("input");
            if(!inputPath.isEmpty()) {
                _params.inputFilepath = inputPath;
            } else {
                syslog(LOG_ERR, "Input file is a required parameter");
                errorCount++;
            }

            /*
            auto outputPath = _parser.value("output");
            if(!outputPath.isEmpty())
                _params.outputFilepath = _parser.value("output");
            else
                _params.outputFilepath = "output";
            */

            auto library = _parser.value("library");
            auto loglevel = _parser.value("loglevel");

            if(loglevel == "debug") {
                _params.logLevel = SupportedLogLevels::debug;
                setlogmask(LOG_UPTO(LOG_DEBUG));
            } else if(loglevel == "info") {
                _params.logLevel = SupportedLogLevels::info;
                setlogmask(LOG_UPTO(LOG_INFO));
            } else if(loglevel == "warn") {
                _params.logLevel = SupportedLogLevels::warn;
                setlogmask(LOG_UPTO(LOG_WARNING));
            } else if(loglevel == "error") {
                _params.logLevel = SupportedLogLevels::error;
                setlogmask(LOG_UPTO(LOG_ERR));
            } else {
                syslog(LOG_ERR, "Unrecognized Log Level: %s", loglevel.toStdString().c_str());
                errorCount++;
            }

            if(library == "ffmpeg") {
                _params.library = SupportedVideoLibraries::FFMpeg;
            } else if(library == "gstreamer") {
                _params.library = SupportedVideoLibraries::GStreamer;
            } else {
                syslog(LOG_ERR, "Unrecognized library: %s", library.toStdString().c_str());
                errorCount++;
            }
        }
    } else {
        syslog(LOG_ERR, "Error parsing command line arguments. Unable to continue");
    }
    return errorCount;
}

int vsg::VideoEditor::Exec() {
    if(_initialized) {
        switch(Configuration().library) {
            case SupportedVideoLibraries::FFMpeg: _ffmVideo = new FFMVideo(); break;
            case SupportedVideoLibraries::GStreamer: _gsVideo = new GSVideo(); break;
            default: _ffmVideo = new FFMVideo(); break;
        }

        if(_ffmVideo) {
            if(_ffmVideo->Wrapper()->Initialize() >= 0) {
                auto ret = _ffmVideo->Wrapper()->LoadSource(_params.inputFilepath.toStdString());
                if (ret >= 0) {
                    syslog(LOG_INFO, "Successfully loaded source (%s) using FFMPEG library", _params.inputFilepath.toStdString().c_str());

                    // TODO: Wrap thread handling into threadpool class to ensure proper thread management
                    _reading = true;
                    _readingLambda = [this]() {
                        if(_ffmVideo) {
                            syslog(LOG_DEBUG, "Executing frame reading thread...");
                            while(_reading)
                                _ffmVideo->Wrapper()->ReadFrame();
                            syslog(LOG_DEBUG, "Exiting frame reading thread...");
                        }
                    };

                    _decoding = true;
                    _decodingLambda = [this]() {
                        if(_ffmVideo) {
                            syslog(LOG_DEBUG, "Executing Frame decoding thread...");
                            while(_decoding)
                                _ffmVideo->Wrapper()->DecodeFrame();
                            syslog(LOG_DEBUG, "Exiting frame decoding thread...");
                        }
                    };

                    _frameDecoderThread = std::thread(_decodingLambda);
                    _frameReaderThread = std::thread(_readingLambda);

                    _frameDecoderThread.detach();
                    _frameReaderThread.detach();
                }
            } else {
                syslog(LOG_ERR, "FFMPEG Video Wrapper failed to initialize correctly");
            }
        }

        if(_gsVideo) {
            if(_gsVideo->Wrapper()->Initialize(_argc, _argv) >= 0) {
                auto ret = _gsVideo->Wrapper()->LoadSource(_params.inputFilepath.toStdString());
                if(ret >= 0) {

                    syslog(LOG_INFO, "Successfully loaded source using GStreamer library");
                } else {
                    syslog(LOG_ERR, "Unable to load source using GStreamer library. Unable to continue");
                }
            } else {
                syslog(LOG_ERR, "GStreamer Video Wrapper failed to initialize correctly");
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    } else {
        syslog(LOG_ERR, "Editor must be initialized before it is used");
    }
    return 0;
}
