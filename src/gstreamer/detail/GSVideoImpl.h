//
// Created by dominic on 16/09/2021.
//

#ifndef VIDEO_EDITOR_GSVIDEOIMPL_H
#define VIDEO_EDITOR_GSVIDEOIMPL_H

#include <gst/gst.h>
#include <string>

namespace vsg {

    class GSEventHandler {
    public:
       static int handle(GstBus * bus, GstMessage * msg, void * data);

       static void onPaddingAdded(GstElement * element, GstPad * padding, void * data);
    };

    class GSVideoImpl {
    public:
        GSVideoImpl();
        ~GSVideoImpl();

        int Initialize(int argc, char * argv[]);
        int LoadSource(const std::string& source);

    protected:
        int Shutdown();

    private:
        std::string _sourcePath;
        GstElement * _pipeline = nullptr;
        GstElement * _source = nullptr;
        GstElement * _demuxer = nullptr;
        GstElement * _decoder = nullptr;
        GstElement * _converter = nullptr;
        GstElement * _sink = nullptr;
        GstBus * _bus = nullptr;
        guint _bus_watch_id;
        bool _initialized = false;
        GMainLoop * _eventLoop = nullptr;
    };
}


#endif //VIDEO_EDITOR_GSVIDEOIMPL_H
