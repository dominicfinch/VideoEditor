
#include "main.h"

namespace fs = std::filesystem;

vsg::VideoEditor * videoEditor = nullptr;

void Shutdown() {
    if(videoEditor) {
        delete videoEditor;
    }
    closelog();
}

void handle_sigint(int sig) {
    Shutdown();
}

void handle_sigquit(int sig) {
    Shutdown();
}

void handle_sigabort(int sig) {
    Shutdown();
}


int main(int argc, char * argv[]) {
    signal(SIGINT, handle_sigint);
    signal(SIGQUIT, handle_sigquit);
    signal(SIGABRT, handle_sigabort);

    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog(PROJECT_SYSLOG_TAG, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
    syslog(LOG_INFO, "Program started by User '%s' (UserId: %d)", getlogin(), getuid());

    int execResult = 0;

    QApplication app(argc, argv);
    QApplication::setApplicationName(PROJECT_NAME);
    QApplication::setApplicationVersion(PROJECT_VERSION);

    videoEditor = new vsg::VideoEditor;
    if(videoEditor->Initialize(argc, argv)) {
        syslog(LOG_INFO, "Successfully initialized Video Editor");
        execResult = videoEditor->Exec();
    } else {
        syslog(LOG_ERR, "Error during initialization of Video Editor. Shutting down...");
    }

    Shutdown();
    return execResult;
}
