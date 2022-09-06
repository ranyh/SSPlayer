#ifndef __PLAYOS_PLAYER_H__
#define __PLAYOS_PLAYER_H__

#include <memory>
#include <functional>

#include "backend/backend.h"


namespace playos {
namespace player {

class ContextExecutor {
public:
    virtual void exec(std::function<void ()> func) = 0;
};

class PlayerCallback {
public:
    virtual void onFrameInfo(std::shared_ptr<VideoFrameInfo> frameInfo) = 0;
    virtual void onFrame(std::shared_ptr<Frame> frame) = 0;
};

class Player: public Handler {
public:
    Player(ContextExecutor *executor);
    ~Player();

    void setExecutor(ContextExecutor *executor) {
        m_executor = executor;
    }

    void setHandler(PlayerCallback *handler) {
        m_handler = handler;
    }

    void runOnRenderThread(std::function<void()> func);

    void setUri(const std::string &uri);
    void play(const std::string &uri);
    void play();
    void pause();
    void stop();
    bool isPlaying();

private:
    void onFrameInfo(std::shared_ptr<VideoFrameInfo> frameInfo);
    void onFrame(std::shared_ptr<Frame> frame);

protected:
    void playBackendCallback(player::CallbackReason reason, void *data) override;

private:
    std::shared_ptr<player::Backend> m_backend;

    ContextExecutor *m_executor;
    PlayerCallback *m_handler;
};

}
}

#endif
