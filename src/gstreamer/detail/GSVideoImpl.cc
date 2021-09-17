//
// Created by dominic on 16/09/2021.
//

#include <syslog.h>
#include <sstream>
#include "GSVideoImpl.h"

vsg::GSVideoImpl::GSVideoImpl() {

}

vsg::GSVideoImpl::~GSVideoImpl() {
    if(_bus)
        gst_object_unref(_bus);

    if(_pipeline) {
        gst_element_set_state(_pipeline, GST_STATE_NULL);
        gst_object_unref(_pipeline);
    }
}

int vsg::GSVideoImpl::Initialize(int argc, char *argv[]) {
    if(!_initialized) {
        gst_init(&argc, &argv);
        _initialized = true;
        return 0;
    } else {
        syslog(LOG_ERR, "Unable to initialize GSVideo more than once");
        return -1;
    }
}

int vsg::GSVideoImpl::LoadSource(const std::string &source) {
    if(_initialized) {
        if(!source.empty()) {
            GError * error;
            std::stringstream ss;
            ss << "playbin uri=" << source;
            _pipeline = gst_parse_launch(ss.str().c_str(), &error);

            if(_pipeline && !error) {
                syslog(LOG_INFO, "Successfully loaded pipeline and source (%s)", source.c_str());
                return 0;
            } else {
                syslog(LOG_ERR, "Error starting GS pipeline with options: %s", error->message);
                gst_object_unref(error);
                return -3;
            }
        } else {
            syslog(LOG_ERR, "Unable to load empty source");
            return -2;
        }
    } else {
        syslog(LOG_ERR, "Unable to load source when not properly initialized");
        return -1;
    }
}



