#include "player.h"

#include <stdio.h>
#include <memory>
#include <unistd.h>

#include "spdlog/spdlog.h"
#include "../eventloop/event_loop_factory.h"
#include "../utils/time.h"


namespace playos {
namespace player {

Player::Player(const char *backend):
        m_backend(player::Backend::create(backend)),
        m_handler(nullptr), m_callbackAudio(false),
        m_videoClock(0), m_audioClock(0), m_eos(false),
        m_currentVFrame(nullptr), m_currentAFrame(nullptr)
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::debug);

    m_eventLoop = std::move(EventLoopFactory::instance().create());
    m_eventLoop->setMainTask(static_cast<Task *>(this));
    m_eventLoop->run(true);

    m_backend->init(static_cast<Handler *>(this));
}

Player::~Player()
{
    m_backend->stop();
    m_eventLoop->stop();
}

void Player::setUri(const std::string &uri)
{
    if (m_state.next(NEW)) {
        m_uri = uri;

        if (m_state.current() == PLAYING || m_state.current() == PAUSED) {
            m_backend->stop();
        }

        m_backend->setUri(m_uri.c_str());
        m_state.commit();
    }
}

void Player::play()
{
    post(MSG_PLAY, 0);
}

void Player::pause()
{
    post(MSG_PAUSE, 0);
}

void Player::stop()
{
    post(MSG_STOP, 0);
}

void Player::seek(int64_t i)
{
    post(MSG_SEEK, i);
}

bool Player::isPlaying()
{
    return m_state.current() == PLAYING;
}

void Player::putFrame(Frame *frame)
{
    m_backend->putFrame(frame);
}

AudioFrame *Player::getAudioFrame()
{
    m_audioMux.lock();
    if (m_audioFrames.empty() || m_state.current() != PLAYING) {
        m_audioMux.unlock();
        return nullptr;
    }

    AudioFrame *frame = (AudioFrame *)m_audioFrames.front();
    m_audioClock = frame->duration;
    m_audioFrames.pop();
    m_audioMux.unlock();

    return frame;
}

void Player::onReady(std::shared_ptr<VideoInfo> &videoInfo, std::shared_ptr<VideoFrameInfo> &frameInfo)
{
    m_videoInfo = videoInfo;
    if (m_handler) {
        m_handler->onReady(videoInfo);
        m_handler->onFrameInfo(frameInfo);
    }
}

void Player::onFrame(Frame *frame)
{
    post(MSG_NEW_FRAME, (uint64_t)frame);
}

void Player::onEOS()
{
    m_eos = true;
}

void Player::onError(const Error &err)
{
    spdlog::error("Backend error: {}", err.msg);
}

EventLoop *Player::getEventLoop()
{
    return m_eventLoop.get();
}

void Player::run(int event)
{
    std::queue<Msg> msgQueue;

    m_mqMux.lock();
    std::swap(m_msgQueue, msgQueue);
    m_mqMux.unlock();

    while (!msgQueue.empty()) {
        Msg msg = msgQueue.front();
        msgQueue.pop();

        switch (msg.type) {
        case MSG_PLAY:
            if (m_state.next(PLAY)) {
                m_state.commit();
                m_backend->play();
            }
            break;
        case MSG_PAUSE:
            if (m_state.next(PAUSE)) {
                m_state.commit();
                m_backend->pause();
            }
            break;
        case MSG_STOP:
            if (m_state.next(STOP)) {
                m_state.commit();
                m_backend->stop();

                clearQueue(m_videoFrames);
                clearQueue(m_audioFrames);
            }
            break;
        case MSG_SEEK:
            m_backend->seek(msg.args);
            break;
        // case MSG_PUT_FRAME: {
        //     Frame *frame = (Frame *)msg.args;
        //     m_backend->putFrame(frame);
        // }
        //     break;
        case MSG_NEW_FRAME: {
            Frame *frame = (Frame *)msg.args;
            if (frame->type == Frame::Video) {
                m_videoFrames.push(frame);
                if (m_state.current() == LOADING) {
                    m_state.next(PLAY);
                    m_state.commit();
                }
            } else if (frame->type == Frame::Audio) {
                m_audioMux.lock();
                m_audioFrames.push(frame);
                m_audioMux.unlock();
            }
        }
            break;
        }
    }

    m_backend->run();

    if (m_state.current() == PLAYING) {
        if (!m_videoFrames.empty()) {
            Frame *frame = m_videoFrames.front();
            if (frame->duration <= m_audioClock) {
                m_handler->onFrame(frame);
                m_videoFrames.pop();
                m_videoClock = frame->duration;

                if (m_currentVFrame != nullptr) {
                    usleep(10000);
                    putFrame(m_currentVFrame);
                }

                m_currentVFrame = frame;
            }
        } else if (m_eos) {
            m_state.next(STOP);
            m_state.commit();
            m_handler->onEOS();
        }

        m_audioMux.lock();
        if (m_callbackAudio && !m_audioFrames.empty()) {
            Frame *frame = m_audioFrames.front();
            m_audioClock += frame->duration;
            m_handler->onFrame(frame);
            m_audioFrames.pop();

            if (m_currentAFrame != nullptr) {
                putFrame(m_currentAFrame);
            }

            m_currentAFrame = frame;
        }
        m_audioMux.unlock();
    }
}

}
}
