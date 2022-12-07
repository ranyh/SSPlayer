#include "audio_sink.h"

namespace playos {

void audioCallback(void *userdata, Uint8 *stream, int len)
{
    auto sink = (AudioSink *)userdata;

    if (sink->m_callback) {
        sink->m_callback(&stream, len);
    };

}

AudioSink::AudioSink(): m_audioDevice(0)
{
    m_desired.freq = 48000;
    m_desired.format = AUDIO_S16SYS;
    m_desired.channels = 2;
    m_desired.samples = 1024;
    m_desired.callback = nullptr;
    m_desired.userdata = this;
}

int AudioSink::init(Callback cb)
{
    deinit();

    m_callback = cb;
    if (cb) {
        m_desired.callback = audioCallback;
    } else {
        m_desired.callback = nullptr;
    }

    m_audioDevice = SDL_OpenAudioDevice(NULL, 0,
            &m_desired, &m_obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (m_audioDevice <= 0) {
        return -1;
    }

    return 0;
}

void AudioSink::deinit()
{
    if (m_audioDevice > 0)
        SDL_CloseAudioDevice(m_audioDevice);
}

void AudioSink::queue(const uint8_t **data, uint32_t len)
{
    if (m_audioDevice > 0)
        SDL_QueueAudio(m_audioDevice, data[0], len);
}

void AudioSink::start()
{
    SDL_PauseAudioDevice(m_audioDevice, 0);
}

void AudioSink::pause()
{
    SDL_PauseAudioDevice(m_audioDevice, 1);
}

bool AudioSink::isStarted()
{
    return SDL_GetAudioDeviceStatus(m_audioDevice) == SDL_AUDIO_PLAYING;
}

}
