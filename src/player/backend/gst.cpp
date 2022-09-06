#include "gst.h"

#include <cstddef>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <thread>
#include "backend.h"
#include "gst/gstelement.h"
#include "gst/gstpad.h"


namespace playos {
namespace player {

/* playbin flags */
typedef enum
{
    GST_PLAY_FLAG_VIDEO = (1 << 0), /* We want video output */
    GST_PLAY_FLAG_AUDIO = (1 << 1), /* We want audio output */
    GST_PLAY_FLAG_TEXT = (1 << 2)   /* We want subtitle output */
} GstPlayFlags;

class GstVideoFrame: public VideoFrame {
public:
    GstVideoFrame(GstBuffer *buffer): m_buffer(buffer), m_maped(false) { }
    ~GstVideoFrame() {
        if (m_maped) {
            unmap();
        }
    }

    bool map(Mode mode) {
        if (m_maped) {
            return true;
        }

        GstMapFlags m = GST_MAP_READ;
        switch (mode) {
        case ReadOnly:
            m = GST_MAP_READ;
            break;
        case WriteOnly:
            m = GST_MAP_WRITE;
            break;
        case ReadWrite:
            m = GST_MAP_FLAG_LAST;
            break;
        }

        if (!gst_buffer_map(m_buffer, &m_map, m)) {
            return false;
        }

        m_maped = true;
        return true;
    };

    bool unmap() {
        if (m_maped) {
            gst_buffer_unmap(m_buffer, &m_map);
            m_maped = false;
        }

        return true;
    };

    size_t loadData(void *data, size_t size) {
        return copyData(data, m_map.data, size);
    }

    void *data() {
        return m_map.data;
    }

    size_t size() {
        return m_map.size;
    }

private:
    GstBuffer *m_buffer;
    GstMapInfo m_map;
    bool m_maped;
};


gboolean GstBackend::handle_message(GstMessage *msg)
{
    GError *err;
    gchar *debug_info;

    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR:
        gst_message_parse_error(msg, &err, &debug_info);
        g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
        g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
        g_clear_error(&err);
        g_free(debug_info);
        break;
    case GST_MESSAGE_EOS:
        g_print("End-Of-Stream reached.\n");
        break;
    case GST_MESSAGE_STATE_CHANGED: {
        GstState old_state, new_state, pending_state;
        gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
        if (GST_MESSAGE_SRC(msg) == GST_OBJECT(playbin)) {
            if (new_state == GST_STATE_PLAYING) {
                /* Once we are in the playing state, analyze the streams */
                // analyze_streams(pSpr);
            }
        }
    }   break;
    default:
        break;
    }

    gst_message_unref(msg);

    return true;
}

GstFlowReturn GstBackend::new_sample_callback(GstAppSink *appsink, gpointer udata)
{
    GstBackend *backend = (GstBackend *)udata;

    GstSample *sample;
    GstBuffer *buffer;

    sample = gst_app_sink_pull_sample(appsink);
    if (!sample) {
        return GST_FLOW_EOS;
    }

    buffer = gst_sample_get_buffer(sample);
    auto f = Frame::create(std::make_shared<GstVideoFrame>(buffer));
    gst_sample_unref(sample);

    backend->m_handler->playBackendCallback(CallbackReason::FRAME, f.get());

    return GST_FLOW_OK;
}

GstFlowReturn GstBackend::new_preroll_callback (GstAppSink *appsink, gpointer udata)
{
    GstBackend *backend = (GstBackend *)udata;
    GstSample *sample;

    sample = gst_app_sink_pull_preroll(appsink);
    if (!sample) {
        return GST_FLOW_EOS;
    }

    GstCaps *caps = gst_sample_get_caps(sample);
    if (!caps) {
        g_print("caps is null.\n");
        return GST_FLOW_EOS;
    }

    GstStructure *capsStruct = gst_caps_get_structure(caps, 0);
    if (capsStruct) {
        // g_print("Structure: %s\n", gst_structure_to_string(capsStruct));
        int width, height;

        gst_structure_get_int(capsStruct, "width", &width);
        gst_structure_get_int(capsStruct, "height", &height);
        const gchar *format = gst_structure_get_string(capsStruct, "format");

        auto info = VideoFrameInfo::create(width, height, RGB);
        backend->m_handler->playBackendCallback(CallbackReason::READY, info.get());
    }

    gst_caps_unref(caps);

    return GST_FLOW_OK;
}

bool GstBackend::doInit()
{
    gint flags;
    GstPad *pad;
    GPollFD busPoolFd;

    setenv("GST_PLUGIN_PATH", "/usr/local/lib/gstreamer-1.0/", 0);

    gst_init(NULL, NULL);

    playbin = gst_element_factory_make("playbin", "playbin");
    if (playbin == nullptr) {
        g_printerr("Playbin element create failed: %s.\n", strerror(errno));
        return false;
    }
    vbin = gst_bin_new("video_sink_bin");
    if (vbin == nullptr) {
        g_printerr("vbin element create failed.\n");
        return false;
    }
    nvconv = gst_element_factory_make("videoconvert", "nvconv");
    if (nvconv == nullptr) {
        g_printerr("nvconv element create failed.\n");
        return false;
    }
    capsfilter = gst_element_factory_make("capsfilter", "filter");
    if (capsfilter == nullptr) {
        g_printerr("capsfilter element create failed.\n");
        return false;
    }

    vsink = gst_element_factory_make("appsink", "video_sink");
    if (vsink == nullptr) {
        g_printerr("vsink element create failed.\n");
        return false;
    }

    caps = gst_caps_new_simple("video/x-raw",
            "format", G_TYPE_STRING, "RGB",
            // "framerate", GST_TYPE_FRACTION, pSpr->info.fps, 1,
            // "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
            // "width", G_TYPE_INT, 320,
            // "height", G_TYPE_INT, 240,
            NULL);
    g_object_set(capsfilter, "caps", caps, NULL); 

    /* Set flags to show Audio and Video but ignore Subtitles */
    g_object_get(playbin, "flags", &flags, NULL);
    flags |= GST_PLAY_FLAG_VIDEO | GST_PLAY_FLAG_AUDIO;
    flags &= ~(GST_PLAY_FLAG_TEXT);
    g_object_set(playbin, "flags", flags, NULL);

    /* Set connection speed. This will affect some internal decisions of playbin */
    g_object_set(playbin, "connection-speed", 56, NULL);

    gst_bin_add_many(GST_BIN(vbin), nvconv, capsfilter, vsink, NULL);
    gst_element_link_many(nvconv, capsfilter, vsink, NULL);

    pad = gst_element_get_static_pad(nvconv, "sink");
    ghost_pad = gst_ghost_pad_new("sink", pad);
    gst_pad_set_active(ghost_pad, TRUE);
    gst_element_add_pad(vbin, ghost_pad);
    gst_object_unref(pad);

    /* Add a bus watch, so we get notified when a message arrives */
    bus = gst_element_get_bus(playbin);
    gst_bus_get_pollfd (bus, &busPoolFd);

    busFd = busPoolFd.fd;
    eventLoop()->addFdWatch(busFd, this, Task::EventIn);

    g_object_set(GST_OBJECT(playbin), "video-sink", vbin, NULL);
    g_object_set(vsink, "emit-signals", TRUE, NULL);
    g_signal_connect(vsink, "new-sample", (GCallback)new_sample_callback, this);
    g_signal_connect(vsink, "new-preroll", (GCallback)new_preroll_callback, this);

    return true;
}

GstBackend::~GstBackend()
{
    eventLoop()->removeFdWatch(busFd);
    terminalEventLoop();

    _stop();

    GstState state;
    gst_element_get_state(playbin, &state, NULL, g_get_monotonic_time () + 100);

    // flush message
    run(0);
    gst_object_unref(bus);
    gst_object_unref(playbin);
}

bool GstBackend::_play()
{
    GstStateChangeReturn ret;

    g_object_set(playbin, "uri", m_uri.c_str(), NULL);

    ret = gst_element_set_state(playbin, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        return -1;
    }

    return true;
}

bool GstBackend::_pause()
{
    return gst_element_set_state(playbin, GST_STATE_PAUSED) != GST_STATE_CHANGE_FAILURE;
}

bool GstBackend::_stop()
{
    gst_element_set_state(playbin, GST_STATE_NULL);
    return true;
}

int GstBackend::_seek(int pos)
{
    return true;
}

void GstBackend::run(int events)
{
    if (isEnd(events)) {
        _stop();
    } else {
        GstMessage *msg = nullptr;
        while ((msg = gst_bus_pop(bus)) != nullptr) {
            handle_message(msg);
        }
    }

}

}
}