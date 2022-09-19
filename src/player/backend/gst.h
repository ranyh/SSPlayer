#ifndef __PLAYOS_PLAYER_GST_H__
#define __PLAYOS_PLAYER_GST_H__

#include "backend.h"

#include <thread>

#include "gst/gst.h"
#include <gst/app/gstappsink.h>
#include <gst/pbutils/pbutils.h>


namespace playos {
namespace player {

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

private:
    virtual bool doInit() override;

    bool onSetUri() override;
    bool onPlay() override;
    bool onPause() override;
    bool onStop() override;
    int onSeek(int64_t pos) override;

private:
    void run(int events) override;
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
