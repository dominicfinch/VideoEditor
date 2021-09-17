//
// Created by dominic on 16/09/2021.
//

#ifndef VIDEO_EDITOR_GSVIDEO_H
#define VIDEO_EDITOR_GSVIDEO_H

#include "interfaces/ivideo.h"
#include "detail/GSVideoImpl.h"

namespace vsg {

    class GSVideo : public IVideo<GSVideoImpl> {
    public:

    private:

    };
}

#endif //VIDEO_EDITOR_GSVIDEO_H
