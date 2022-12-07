#ifndef __PLAYOS_AUDIO_SINK_H__
#define __PLAYOS_AUDIO_SINK_H__

#include "frame.h"

#include <SDL.h>

#include <functional>


namespace playos {

class AudioSink {
public:
    using Callback = std::function<void(uint8_t **stream, int len)>;
    friend void audioCallback(void *userdata, Uint8 *stream, int len);

public:
    AudioSink();

    int init(Callback cb = nullptr);
    void deinit();
    void queue(const uint8_t **data, uint32_t len);
    void start();
    void pause();

    bool isStarted();

private:
    AudioSink(const AudioSink&) = delete;
    AudioSink(AudioSink&&) = delete;

private:
    Callback m_callback;
    SDL_AudioDeviceID m_audioDevice;
    SDL_AudioSpec m_desired;
    SDL_AudioSpec m_obtained;
};

}

#endif
