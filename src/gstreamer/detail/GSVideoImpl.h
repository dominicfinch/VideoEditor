//
// Created by dominic on 16/09/2021.
//

#ifndef VIDEO_EDITOR_GSVIDEOIMPL_H
#define VIDEO_EDITOR_GSVIDEOIMPL_H

#include <gst/gst.h>
#include <string>

namespace vsg {
    class GSVideoImpl {
    public:
        GSVideoImpl();
        ~GSVideoImpl();

        int Initialize(int argc, char * argv[]);
        int LoadSource(const std::string& source);

    private:
        GstElement * _pipeline = nullptr;
        GstBus * _bus = nullptr;
        bool _initialized = false;
    };
}


#endif //VIDEO_EDITOR_GSVIDEOIMPL_H
