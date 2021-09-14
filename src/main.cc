

#include <iostream>
#include "main.h"

#include <unistd.h>
#include <syslog.h>


int main(int argc, char * argv[]) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog(PROJECT_SYSLOG_TAG, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
    syslog(LOG_INFO, "Program started by User '%s' (UserId: %d)", getlogin(), getuid());

    FFMVideo video;
    if(argc > 1) {
        if(video.Initialize()) {
            auto inputFile = argv[1];
            if(video.LoadMedia(inputFile)) {

            } else {
                syslog(LOG_ERR, "Unable to load input file: %s", inputFile);
            }
        } else {
            syslog(LOG_ERR, "Failed to initialize properly");
        }
    }
    closelog();
    return 0;
}
