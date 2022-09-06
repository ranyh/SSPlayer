#ifndef __PLAYOS_PLAYER_GST_H__
#define __PLAYOS_PLAYER_GST_H__

#include "backend.h"

#include <thread>

#include "gst/gst.h"
#include <gst/app/gstappsink.h>


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
    int busFd;

private:
    virtual bool doInit() override;

    bool _play() override;
    bool _pause() override;
    bool _stop() override;
    int _seek(int pos) override;

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

public:
    static GstFlowReturn new_sample_callback(GstAppSink *appsink, gpointer udata);
    static GstFlowReturn new_preroll_callback (GstAppSink *appsink, gpointer user_data);
};

}
}

#endif
