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
#include <vector>

#include "../../eventloop/select_event_loop.h"
#include "frame.h"

namespace playos {
namespace player {

class Handler;

class Backend {
public:
    enum State {
        CREARED,
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

        post<bool>([&]() {
            onSetUri();
        });
    }

    std::string uri() {
        return m_uri;
    }

    void onError(const std::string &msg);

    virtual ~Backend();

    bool play(const std::string &uri);
    bool play();
    bool pause();
    bool stop();
    int seek(int64_t pos);

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
    virtual bool onSetUri() = 0;
    virtual bool onPlay() = 0;
    virtual bool onPause() = 0;
    virtual bool onStop() = 0;
    virtual int onSeek(int64_t pos) = 0;
};

enum CallbackReason {
    READY,
    FRAME,
    ERROR,
    STATE_CHANGED,
    EOS,
};

class Error: public std::enable_shared_from_this<Error> {
private:
    Error(const std::string &msg): msg(msg) { }
public:
    static std::shared_ptr<Error> create(const std::string &msg) {
        return std::shared_ptr<Error>(new Error(msg));
    }

public:
    const std::string msg;
};

class StateChanged {
public:
    StateChanged(Backend::State state): state(state) { }
    StateChanged(Backend::State state, void *data): state(state), data(data) { }

    void set(void *data) {
        this->data = data;
    }

    template<typename T>
    T get() {
        return reinterpret_cast<T>(data);
    }

public:
    Backend::State state;

private:
    void *data;
};

class VideoInfo: public std::enable_shared_from_this<VideoInfo> {
public:
    struct Tag {
        std::string title;
        std::string videoCodec;
        std::string composer;
        std::string artist;
        std::string comment;
        std::string encoder;
        std::string genre;
        std::string containerFormat;
        uint64_t bitrate;

        Tag() { }
        Tag(const std::string &title,
            const std::string &videoCodec,
            const std::string &composer,
            const std::string &artist,
            const std::string &comment,
            const std::string &encoder,
            const std::string &genre,
            const std::string &containerFormat,
            uint64_t bitrate):
                title(title),
                videoCodec(videoCodec),
                composer(composer),
                artist(artist),
                comment(comment),
                encoder(encoder),
                genre(genre),
                containerFormat(containerFormat),
                bitrate(bitrate) { }
    };

private:
    VideoInfo() { }

public:
    static std::shared_ptr<VideoInfo> create() {
        return std::shared_ptr<VideoInfo>(new VideoInfo());
    }

public:
    std::vector<const Tag> tags;
    uint64_t duration;
    bool seekable;
};

class Handler {
public:
    virtual void playBackendCallback(CallbackReason reason, void *data) = 0;
};


}
}

#endif
