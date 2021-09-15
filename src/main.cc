

#include <iostream>
#include "main.h"
#include "FFMUtils.h"

#include <unistd.h>
#include <syslog.h>
#include <filesystem>

namespace fs = std::filesystem;


int main(int argc, char * argv[]) {
    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog(PROJECT_SYSLOG_TAG, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
    syslog(LOG_INFO, "Program started by User '%s' (UserId: %d)", getlogin(), getuid());

    FFMVideo video;
    if(argc > 1) {
        if(video.Initialize()) {
            auto inputFile = argv[1];
            if(video.LoadMedia(inputFile)) {
                auto v = video.Video();

                std::string outputDir = "./output";
                if(!fs::exists(outputDir)) {
                    fs::create_directory(outputDir);
                }

                for(auto i=0; i<v.size(); i++) {
                    // Once every 30 frames save a picture
                    if(i % 25 == 0) {
                        std::stringstream ss;
                        ss << outputDir << "/" << "image_" << i << ".pmg";
                        auto ret = utils::draw_grayscale_image(ss.str(), v[i]);
                        if(ret <= 0) {

                        }

                        ss.clear();
                    }
                }
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
