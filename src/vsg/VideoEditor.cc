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
    if(_ffmVideo)
        delete _ffmVideo;

    if(_gsVideo)
        delete _gsVideo;
}

bool vsg::VideoEditor::Initialize(int argc, char * argv[]) {
    auto errorCount = processArguments(argc, argv);
    if(errorCount == 0)
        _initialized = true;
    return _initialized;
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
                    syslog(LOG_INFO, "Successfully loaded source using FFMPEG library");
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
                }
            } else {
                syslog(LOG_ERR, "GStreamer Video Wrapper failed to initialize correctly");
            }
        }

    } else {
        syslog(LOG_ERR, "Editor must be initialized before it is used");
    }
    return 0;
}
