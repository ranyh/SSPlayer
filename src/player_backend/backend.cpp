#include "backend.h"

#include <memory>

#ifdef GST_BACKEND
#include "backend/gst.h"
#endif

#ifdef FFMPEG_BACKEND
#include "backend/ffmpeg.h"
#endif


namespace playos {
namespace player {

std::unique_ptr<Backend> Backend::create(const std::string &name)
{
    std::unique_ptr<Backend> backend;

    if (name == "gst") {
#ifdef GST_BACKEND
        backend.reset(new GstBackend());
#endif
    } else if (name == "ffmpeg") {
#ifdef FFMPEG_BACKEND
        backend.reset(new FFmpegBackend());
#endif
    }

    return std::move(backend);
}

Backend::~Backend()
{
    m_handler = nullptr;
}

void Backend::onError(const std::string &msg)
{
    m_handler->onError(Error(msg.c_str()));
}

void Backend::setUri(const char *uri)
{
    strncpy(m_uri, uri, sizeof(m_uri));
}

}
}
