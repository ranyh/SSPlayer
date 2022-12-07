#ifndef __PLAYOS_PLAYER_GST_H__
#define __PLAYOS_PLAYER_GST_H__

#include "backend.h"

#include <thread>

#include "gst/gst.h"
#include <gst/app/gstappsink.h>
#include <gst/pbutils/pbutils.h>

#include "frame_pool.h"
#include "spdlog/spdlog.h"


namespace playos {
namespace player {

class GstVideoFrame: public VideoFrame {
public:
    GstVideoFrame(std::shared_ptr<VideoFrameInfo> info, GstBuffer *buffer):
            VideoFrame(info), m_buffer(buffer) {
        gst_buffer_ref(buffer);

        duration = 0;
        if (gst_buffer_map(m_buffer, &m_map, GST_MAP_READ)) {
            data[0] = m_map.data;
            size[0] = m_map.size;
            stride[0] = 0;
            offset[0] = 0;
        } else {
            spdlog::error("Failed to map memory for read.");
        }
    }
    ~GstVideoFrame() {
        gst_buffer_unmap(m_buffer, &m_map);
        gst_buffer_unref(m_buffer);
    }

private:
    GstBuffer *m_buffer;
    GstMapInfo m_map;
};

class GstBackend: public Backend, public Task
{
private:
    GstBus *bus;
    GstPad *ghost_pad;
    GstCaps *caps;

    GstElement *playbin;
    GstElement *nvconv;
    GstElement *vbin;
    GstElement *capsfilter;
    GstElement *vsink;
    GstDiscoverer *discoverer;
    int busFd;

    std::shared_ptr<VideoInfo> m_videoInfo;
    std::shared_ptr<VideoFrameInfo> m_videoFrameInfo;

    std::unique_ptr<FramePool<GstVideoFrame>> m_videoPool;

private:
    virtual bool doInit() override;

    bool play() override;
    bool pause() override;
    bool stop() override;
    int seek(int64_t pos) override;
    void putFrame(Frame *frame) override;

    void run(int event) override;

private:
    bool initPlay();
    gboolean handle_message(GstMessage *msg);

public:
    GstBackend():
        bus(nullptr),
        ghost_pad(nullptr),
        caps(nullptr),
        playbin(nullptr),
        nvconv(nullptr),
        vbin(nullptr),
        capsfilter(nullptr),
        vsink(nullptr) { }
    ~GstBackend();

    void mediaDiscovered();
    void mediaDiscoverFinished();

public:
    static GstFlowReturn new_sample_callback(GstAppSink *appsink, gpointer udata);
    static GstFlowReturn new_preroll_callback (GstAppSink *appsink, gpointer user_data);
};

}
}

#endif
