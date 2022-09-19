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

void Backend::onError(const std::string &msg)
{
    m_handler->playBackendCallback(CallbackReason::ERROR, Error::create(msg).get());
}

bool Backend::play(const std::string &uri)
{
    setUri(uri);
    return play();
}

bool Backend::play()
{
    post<bool>([&]() {
        onPlay();
        m_state = Backend::PLAYING;
    });

    return true;
}


bool Backend::pause()
{
    post<bool>([&]() {
        onPause();
        m_state = Backend::PAUSED;
    });

    return true;
}

bool Backend::stop()
{
    post<bool>([&]() {
        onStop();
        m_state = Backend::STOPED;
    });

    return true;
}

int Backend::seek(int64_t pos)
{
    post<bool>([&, pos]() {
        onSeek(pos);
    });

    return 0;
}

bool Backend::isPlaying()
{
    return m_state == PLAYING;
}

}
}
