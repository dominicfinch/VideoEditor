//
// Created by dominic on 16/09/2021.
//

#ifndef VIDEO_EDITOR_IVIDEO_H
#define VIDEO_EDITOR_IVIDEO_H

namespace vsg {
    template <class VideoWrapper>
    class IVideo {
    public:
        IVideo(): _video(new VideoWrapper) {}
        ~IVideo() {
            if(_video)
                delete _video;
        }

        VideoWrapper * Wrapper() { return _video; }

    protected:
        VideoWrapper * _video = nullptr;
    };
}

#endif //VIDEO_EDITOR_IVIDEO_H
