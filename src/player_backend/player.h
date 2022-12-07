#ifndef __PLAYOS_PLAYER_H__
#define __PLAYOS_PLAYER_H__

#include <memory>
#include <functional>
#include <queue>

#include "backend.h"
#include "state.h"
#include "../eventloop/event_loop.h"


namespace playos {
namespace player {

const int MAX_QUEUE_SIZE = 128;

class PlayerCallback {
public:
    virtual void onReady(std::shared_ptr<VideoInfo> info) = 0;
    virtual void onEOS() = 0;
    virtual void onFrameInfo(std::shared_ptr<VideoFrameInfo> frameInfo) = 0;
    virtual void onFrame(Frame *frame) = 0;
};

class Player: public Handler, public Task {
private:
    enum MsgType {
        MSG_PLAY,
        MSG_STOP,
        MSG_PAUSE,
        MSG_SEEK,
        MSG_PUT_FRAME,
        MSG_NEW_FRAME,
    };

    struct Msg {
        MsgType type;
        int64_t args;

        Msg() { }
        Msg(MsgType type, int64_t args): type(type), args(args) {}
    };

public:
    Player(const char *backend);
    ~Player();

    void setHandler(PlayerCallback *handler) {
        m_handler = handler;
    }

    void setUri(const std::string &uri);
    void play();
    void pause();
    void stop();
    void seek(int64_t i);
    bool isPlaying();
    void putFrame(Frame *frame);

    void setAudioFromCallback(bool f) {
        m_callbackAudio = f;
    }

    AudioFrame *getAudioFrame();

private:
    void onReady(std::shared_ptr<VideoInfo> &videoInfo, std::shared_ptr<VideoFrameInfo> &frameInfo) override;
    void onFrame(Frame *frame) override;
    void onEOS() override;
    void onError(const Error &err) override;
    EventLoop *getEventLoop() override;

    void run(int event) override;

private:
    void post(MsgType type, uint64_t args) {
        m_mqMux.lock();
        m_msgQueue.emplace(type, args);
        m_mqMux.unlock();
    }

    template<typename T>
    void clearQueue(std::queue<T> &que) {
        std::queue<T> empty;
        std::swap(que, empty);
    }

private:
    std::unique_ptr<Backend> m_backend;
    std::shared_ptr<VideoInfo> m_videoInfo;
    std::shared_ptr<EventLoop> m_eventLoop;
    std::string m_uri;

    Fsm m_state;

    PlayerCallback *m_handler;
    std::mutex m_mqMux;
    std::queue<Msg> m_msgQueue;

    std::queue<Frame *> m_videoFrames;
    std::queue<Frame *> m_audioFrames;
    Frame *m_currentVFrame;
    Frame *m_currentAFrame;
    bool m_callbackAudio;
    std::mutex m_audioMux;
    int64_t m_videoClock;
    int64_t m_audioClock;
    bool m_eos;
};

}
}

#endif
