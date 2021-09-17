//
// Created by dominic on 16/09/2021.
//

#ifndef VIDEO_EDITOR_FFMVIDEO_H
#define VIDEO_EDITOR_FFMVIDEO_H

#include "interfaces/ivideo.h"
#include "detail/FFMVideoImpl.h"

namespace vsg {

    class FFMVideo: public IVideo<FFMVideoImpl> {
    public:
        FFMVideo();
        ~FFMVideo();

    private:

    };
}


#endif //VIDEO_EDITOR_FFMVIDEO_H
