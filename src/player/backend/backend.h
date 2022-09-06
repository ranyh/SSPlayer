#ifndef __PLAYOS_PLAYER_BACKEND_H__
#define __PLAYOS_PLAYER_BACKEND_H__

#include <stddef.h>
#include <stdio.h>
#include <stddef.h>

#include <functional>
#include <future>
#include <memory>
#include <string.h>
#include <string>

#include "../../eventloop/select_event_loop.h"
#include "frame.h"

namespace playos {
namespace player {

enum CallbackReason {
    FRAME,
    READY,
    PLAYER,
    STOP,
    PAUSE,
    ERROR,
};

class Handler {
public:
    virtual void playBackendCallback(CallbackReason reason, void *data) = 0;
};

class Backend {
public:
    enum State {
        CREARED,
        READY,
        PLAYING,
        PAUSED,
        STOPED,
    };

protected:
    std::string m_uri;
    Handler *m_handler;
    std::shared_ptr<EventLoop> m_eventLoop;

    State m_state;

public:
    static std::shared_ptr<Backend> create(const std::string &name);

public:
    Backend(): m_handler(nullptr), m_state(CREARED) {
        m_eventLoop = std::move(EventLoopFactory::instance().create());
        m_eventLoop->run(true);
    }

    std::shared_ptr<EventLoop> eventLoop() {
        return m_eventLoop;
    }

    bool init(Handler *hander) {
        m_handler = hander;
        return doInit();
    }

    void setUri(const std::string &uri) {
        if (m_uri != uri) {
            m_uri = uri;
        }
    }

    std::string uri() {
        return m_uri;
    }

    virtual ~Backend();

    bool play(const std::string &uri);
    bool play();
    bool pause();
    bool stop();
    int seek(int pos);

    bool isPlaying();

private:
    template<typename T>
    std::future<T> post(std::function<void()> func) {
        if (!m_eventLoop) {
            return std::future<T>();
        }

        auto *p = new std::promise<T>();

        m_eventLoop->postTask(Task::create([p, func](Task *task, int events) {
            if (!Task::isEnd(events)) {
                func();
            }

            delete p;
            delete task;
        }));

        return p->get_future();
    }

protected:
    /**
     * @brief Decontructor of derived class should call this function first.
     * 
     */
    void terminalEventLoop() {
        m_eventLoop->stop();
    }

protected:
    virtual bool doInit() { return true; };
    virtual bool _play() = 0;
    virtual bool _pause() = 0;
    virtual bool _stop() = 0;
    virtual int _seek(int pos) = 0;
};

}
}

#endif
