//
// Created by dominic on 16/09/2021.
//

#include <syslog.h>
#include <sstream>
#include "GSVideoImpl.h"

vsg::GSVideoImpl::GSVideoImpl() {

}

vsg::GSVideoImpl::~GSVideoImpl() {
    Shutdown();
}

int vsg::GSVideoImpl::Initialize(int argc, char *argv[]) {
    if(!_initialized) {
        gst_init(&argc, &argv);

        _pipeline = gst_pipeline_new ("video-player");
        _source = gst_element_factory_make ("filesrc","file-source");
        _demuxer = gst_element_factory_make ("oggdemux","ogg-demuxer");
        _decoder = gst_element_factory_make ("vorbisdec","vorbis-decoder");
        _converter = gst_element_factory_make ("audioconvert", "converter");
        _sink = gst_element_factory_make ("autovideosink", "video-output");
        _eventLoop = g_main_loop_new(nullptr, false);

        if(_pipeline && _source && _demuxer && _decoder && _converter && _sink && _eventLoop) {
            syslog(LOG_INFO, "Successfully initialized GStreamer pipeline elements");
            _initialized = true;
            return 0;
        } else {
            syslog(LOG_ERR, "Error initializing GStreamer pipeline elements");
            return -2;
        }
    } else {
        syslog(LOG_ERR, "Unable to initialize GSVideo more than once");
        return -1;
    }
}

int vsg::GSVideoImpl::LoadSource(const std::string &source) {
    if(_initialized) {
        if(!source.empty()) {
            ///////////////////////////
            /* Set Source element */
            g_object_set(G_OBJECT(_source), "location", source.c_str(), nullptr);

            /* Add message handler */
            _bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline));
            _bus_watch_id = gst_bus_add_watch(_bus, vsg::GSEventHandler::handle, _eventLoop);
            gst_object_unref(_bus);

            /* we add all elements into the pipeline */
            /* file-source | ogg-demuxer | vorbis-decoder | converter | alsa-output */
            gst_bin_add_many(GST_BIN(_pipeline), _source, _demuxer, _decoder, _converter, _sink, nullptr);

            /* we link the elements together */
            /* file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output */
            gst_element_link(_source, _demuxer);
            gst_element_link_many(_decoder, _converter, _sink, nullptr);
            g_signal_connect(_demuxer, "pad-added", G_CALLBACK(vsg::GSEventHandler::onPaddingAdded), _decoder);

            _sourcePath = source;
            ///////////////////////////
            return 0;
        } else {
            syslog(LOG_ERR, "Unable to load empty source");
            return -2;
        }
    } else {
        syslog(LOG_ERR, "Unable to load source when not properly initialized");
        return -1;
    }
}

int vsg::GSVideoImpl::Shutdown() {

    if(_eventLoop) {
        g_main_loop_unref(_eventLoop);
    }

    if(_pipeline) {
        gst_element_set_state(_pipeline, GST_STATE_NULL);
        gst_object_unref(_pipeline);
        _pipeline = nullptr;
    }

    g_source_remove(_bus_watch_id);

    if(_bus) {
        gst_object_unref(_bus);
    }

    if(_converter) {
        gst_element_set_state(_converter, GST_STATE_NULL);
        gst_object_unref(_converter);
        _converter = nullptr;
    }

    if(_sink) {
        gst_element_set_state(_sink, GST_STATE_NULL);
        gst_object_unref(_sink);
        _sink = nullptr;
    }

    if(_decoder) {
        gst_element_set_state(_decoder, GST_STATE_NULL);
        gst_object_unref(_decoder);
        _decoder = nullptr;
    }

    if(_demuxer) {
        gst_element_set_state(_demuxer, GST_STATE_NULL);
        gst_object_unref(_demuxer);
        _demuxer = nullptr;
    }

    if(_source) {
        gst_element_set_state(_source, GST_STATE_NULL);
        gst_object_unref(_source);
        _source = nullptr;
    }

    return 0;
}

int vsg::GSEventHandler::handle(GstBus *bus, GstMessage *msg, void *data) {
    auto * loop = (GMainLoop *) data;

    switch(GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_EOS:
            g_print("End of stream\n");
            g_main_loop_quit(loop);
            break;
        case GST_MESSAGE_ERROR: {
            gchar  *debug;
            GError *error;

            gst_message_parse_error (msg, &error, &debug);
            g_free(debug);

            g_printerr("Error: %s\n", error->message);
            g_error_free(error);
            g_main_loop_quit(loop);
            break;
        }
        default:
            break;
    }

    return true;
}

void vsg::GSEventHandler::onPaddingAdded(GstElement * element, GstPad * padding, void *data) {
    GstPad * sinkpad;
    auto * decoder = (GstElement *)data;
    g_print("Dynamic pad created, linking demuxer/decoder\n");
    sinkpad = gst_element_get_static_pad(decoder, "sink");
    gst_pad_link(padding, sinkpad);
    gst_object_unref(sinkpad);
}
