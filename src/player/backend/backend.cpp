#include "backend.h"

#include <memory>

#include "gst.h"


namespace playos {
namespace player {

std::shared_ptr<Backend> Backend::create(const std::string &name)
{
    std::shared_ptr<Backend> backend;

    if (name == "gst") {
        backend = std::make_shared<GstBackend>();
    }

    return backend;
}

Backend::~Backend()
{
    m_handler = nullptr;
}

bool Backend::play(const std::string &uri)
{
    setUri(uri);
    return play();
}

bool Backend::play()
{
    post<bool>([&]() {
        _play();
        m_state = Backend::PLAYING;
    });

    return true;
}


bool Backend::pause()
{
    post<bool>([&]() {
        _pause();
        m_state = Backend::PAUSED;
    });

    return true;
}

bool Backend::stop()
{
    post<bool>([&]() {
        _stop();
        m_state = Backend::STOPED;
    });

    return true;
}

int Backend::seek(int pos)
{
    post<bool>([&, pos]() {
        _seek(pos);
    });

    return 0;
}

bool Backend::isPlaying()
{
    return m_state == PLAYING;
}

}
}
