#include "player.h"

#include <stdio.h>
#include <memory>

#include "backend/backend.h"

namespace playos {
namespace player {

Player::Player(ContextExecutor *executor):
        m_backend(player::Backend::create("gst")),
        m_executor(executor),
        m_handler(nullptr)
{
    m_backend->init(this);
}

Player::~Player()
{
    m_backend->stop();
}

void Player::setUri(const std::string &uri)
{
    m_backend->setUri(uri);
}

void Player::play(const std::string &uri)
{
    m_backend->play(uri);
}

void Player::play()
{
    m_backend->play();
}

void Player::pause()
{
    m_backend->pause();
}

void Player::stop()
{
    m_backend->stop();
}

bool Player::isPlaying()
{
    return m_backend->isPlaying();
}

void Player::onFrameInfo(std::shared_ptr<player::VideoFrameInfo> frameInfo)
{
    if (m_handler)
        m_handler->onFrameInfo(frameInfo);
}

void Player::onFrame(std::shared_ptr<player::Frame> frame)
{
    if (m_handler)
        m_handler->onFrame(frame);
}

void Player::playBackendCallback(player::CallbackReason reason, void *data)
{
    switch (reason) {
    case player::READY: {
        auto info = reinterpret_cast<player::VideoFrameInfo *>(data);
        runOnRenderThread(std::bind(&Player::onFrameInfo, this, info->shared_from_this()));
    }   break;
    case player::FRAME: {
        auto frame = reinterpret_cast<player::Frame *>(data);
        runOnRenderThread(std::bind(&Player::onFrame, this, frame->shared_from_this()));
    }   break;
    default:
        break;
    }
}

void Player::runOnRenderThread(std::function<void()> func)
{
    if (m_executor)
        m_executor->exec(func);
}

}
}
