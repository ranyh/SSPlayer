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
        m_handler->onEOS();
        break;
    case GST_MESSAGE_STATE_CHANGED: {
        // GstState old_state, new_state, pending_state;
        // gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
        // if (GST_MESSAGE_SRC(msg) == GST_OBJECT(playbin)) {
        //     if (new_state == GST_STATE_PLAYING) {
        //         StateChanged state(PLAYING, nullptr);
        //         m_handler->playBackendCallback(CallbackReason::STATE_CHANGED, &state);
        //     } else if (new_state == GST_STATE_PAUSED) {
        //         StateChanged state(PAUSED, nullptr);
        //         m_handler->playBackendCallback(CallbackReason::STATE_CHANGED, &state);
        //     } else if (new_state == GST_STATE_NULL) {
        //         StateChanged state(STOPED, nullptr);
        //         m_handler->playBackendCallback(CallbackReason::STATE_CHANGED, &state);
        //     } else if (new_state == GST_STATE_READY) {
        //     } else {
        //         printf("GST_STATE_UNKOWN\n");
        //     }
        // }
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
    auto f = new GstVideoFrame(backend->m_videoFrameInfo, buffer);
    gst_sample_unref(sample);

    gst_element_query_position(backend->playbin, GST_FORMAT_TIME, &f->duration);
    f->duration /= 1000000;
    backend->m_handler->onFrame(f);

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

        backend->m_videoFrameInfo = VideoFrameInfo::create(width, height, RGB);
        backend->m_handler->onReady(backend->m_videoInfo, backend->m_videoFrameInfo);
    }

    gst_caps_unref(caps);

    return GST_FLOW_OK;
}

bool GstBackend::doInit()
{
    gint flags;
    GstPad *pad;
    GPollFD busPoolFd;
    GError *err = nullptr;

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
    gst_bus_get_pollfd(bus, &busPoolFd);

    busFd = busPoolFd.fd;
    m_handler->getEventLoop()->addFdWatch(busFd, this, Task::EventIn);

    g_object_set(GST_OBJECT(playbin), "video-sink", vbin, NULL);
    g_object_set(vsink, "emit-signals", TRUE, NULL);
    g_signal_connect(vsink, "new-sample", (GCallback)new_sample_callback, this);
    g_signal_connect(vsink, "new-preroll", (GCallback)new_preroll_callback, this);

    discoverer = gst_discoverer_new (3 * GST_SECOND, &err);
    if (!discoverer) {
        g_print ("Error creating discoverer instance: %s\n", err->message);
        g_clear_error (&err);
        return false;
    }

    return true;
}

GstBackend::~GstBackend()
{
    m_handler->getEventLoop()->removeFdWatch(busFd);

    GstState state;
    gst_element_get_state(playbin, &state, NULL, g_get_monotonic_time () + 100);

    g_object_unref (discoverer);
    gst_object_unref(bus);
    gst_object_unref(playbin);
}

bool GstBackend::initPlay()
{
    GError *err = nullptr;
    std::string uri = m_uri;
    if (uri.find("://") == uri.npos) {
        uri = "file://" + uri;
    }

    g_object_set(playbin, "uri", uri.c_str(), NULL);

    GstDiscovererInfo *info = gst_discoverer_discover_uri(discoverer, uri.c_str(), &err);
    if (info == nullptr) {
        g_print("Failed to start discovering URI '%s': %s\n", uri.c_str(), err->message);
        onError(err->message);
        g_clear_error(&err);
        return false;
    } else {
        GstDiscovererResult result;
        const gchar *uri;
        const GstTagList *tags;
        GstDiscovererStreamInfo *sinfo;

        uri = gst_discoverer_info_get_uri(info);
        result = gst_discoverer_info_get_result(info);
        switch (result) {
        case GST_DISCOVERER_URI_INVALID:
            g_print("Invalid URI '%s'\n", uri);
            break;
        case GST_DISCOVERER_ERROR:
            g_print("Discoverer error: %s\n", err->message);
            break;
        case GST_DISCOVERER_TIMEOUT:
            g_print("Timeout\n");
            break;
        case GST_DISCOVERER_BUSY:
            g_print("Busy\n");
            break;
        case GST_DISCOVERER_MISSING_PLUGINS:{
            const GstStructure *s;
            gchar *str;

            s = gst_discoverer_info_get_misc(info);
            str = gst_structure_to_string(s);

            g_print("Missing plugins: %s\n", str);
            g_free(str);
            break;
            }
        case GST_DISCOVERER_OK:
            break;
        }

        if (result != GST_DISCOVERER_OK) {
            g_printerr("This URI cannot be played\n");
            return false;
        }

        m_videoInfo = VideoInfo::create();
        m_videoInfo->duration = gst_discoverer_info_get_duration(info)/1000000;
        m_videoInfo->seekable = gst_discoverer_info_get_seekable(info);
        m_videoInfo->tags.emplace_back();

        tags = gst_discoverer_info_get_tags (info);
        if (tags) {
            gst_tag_list_foreach(tags, [](const GstTagList *tags, const gchar *tag, gpointer user_data) {
                GValue val = { 0, };
                gchar *str;
                VideoInfo::Tag *_tag = static_cast<VideoInfo::Tag *>(user_data);

                gst_tag_list_copy_value (&val, tags, tag);

                if (G_VALUE_HOLDS_STRING (&val))
                    str = g_value_dup_string (&val);
                else
                    str = gst_value_serialize (&val);

                if (strcmp("video codec", gst_tag_get_nick(tag)) == 0) {
                    _tag->videoCodec = str;
                } else if (strcmp("bitrate", gst_tag_get_nick(tag)) == 0) {
                    if (G_VALUE_HOLDS(&val, G_TYPE_FLOAT)) {
                        _tag->bitrate = g_value_get_float(&val);
                    } else if (G_VALUE_HOLDS(&val, G_TYPE_INT)) {
                        _tag->bitrate = g_value_get_int(&val);
                    } else if (G_VALUE_HOLDS(&val, G_TYPE_INT64)) {
                        _tag->bitrate = g_value_get_int64(&val);
                    } else if (G_VALUE_HOLDS(&val, G_TYPE_LONG)) {
                        _tag->bitrate = g_value_get_long(&val);
                    }
                } else if (strcmp("title", gst_tag_get_nick(tag)) == 0) {
                    _tag->title = str;
                } else if (strcmp("composer", gst_tag_get_nick(tag)) == 0) {
                    _tag->composer = str;
                } else if (strcmp("artist", gst_tag_get_nick(tag)) == 0) {
                    _tag->artist = str;
                } else if (strcmp("comment", gst_tag_get_nick(tag)) == 0) {
                    _tag->comment = str;
                } else if (strcmp("encoder", gst_tag_get_nick(tag)) == 0) {
                    _tag->encoder = str;
                } else if (strcmp("genre", gst_tag_get_nick(tag)) == 0) {
                    _tag->genre = str;
                } else if (strcmp("container format", gst_tag_get_nick(tag)) == 0) {
                    _tag->containerFormat = str;
                }

                // g_print ("%*s%s: %s\n", 2, " ", gst_tag_get_nick (tag), str);
                g_free (str);

                g_value_unset (&val);
            }, (void *)&m_videoInfo->tags[0]);
        }
    }

    return true;
}

bool GstBackend::play()
{
    GstStateChangeReturn ret;

    initPlay();
    ret = gst_element_set_state(playbin, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        onError("Unable to set the pipeline to the playing state.\n");
        return false;
    }

    return true;
}

bool GstBackend::pause()
{
    return gst_element_set_state(playbin, GST_STATE_PAUSED) != GST_STATE_CHANGE_FAILURE;
}

bool GstBackend::stop()
{
    gst_element_set_state(playbin, GST_STATE_NULL);
    return true;
}

int GstBackend::seek(int64_t pos)
{
    gst_element_seek_simple(playbin, GST_FORMAT_TIME,
            GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE), pos);
    return 0;
}

void GstBackend::putFrame(Frame *frame)
{
    delete frame;
}

void GstBackend::run(int event)
{
    GstMessage *msg = nullptr;
    while ((msg = gst_bus_pop(bus)) != nullptr) {
        handle_message(msg);
    }
}

}
}