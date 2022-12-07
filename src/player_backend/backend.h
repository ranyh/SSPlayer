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

#include "frame.h"
#include "../eventloop/event_loop.h"

namespace playos {
namespace player {

class Handler;

class Backend {
public:
    static std::unique_ptr<Backend> create(const std::string &name);

public:
    Backend(): m_handler(nullptr) {
    }

    bool init(Handler *hander) {
        m_handler = hander;
        return doInit();
    }

    void onError(const std::string &msg);

    virtual ~Backend();

    virtual void setUri(const char *uri);
    virtual bool play() = 0;
    virtual bool pause() = 0;
    virtual bool stop() = 0;
    virtual int seek(int64_t pos) = 0;

    virtual void putFrame(Frame *frame) = 0;
    virtual void run() { }

protected:
    virtual bool doInit() { return true; };

protected:
    char m_uri[265];
    Handler *m_handler;
};

class Error {
public:
    Error(const char *msg) {
        strncpy(this->msg, msg, 256);
    }

    Error(const Error &) = delete;
    Error(Error &&) = delete;

public:
    char msg[265];
};

class VideoInfo {
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
    VideoInfo(const VideoInfo&) = delete;
    VideoInfo(VideoInfo&&) = delete;

public:
    static std::shared_ptr<VideoInfo> create() {
        return std::shared_ptr<VideoInfo>(new VideoInfo());
    }

public:
    std::vector<Tag> tags;
    uint64_t duration;
    bool seekable;
};

class Handler {
public:
    virtual void onReady(std::shared_ptr<VideoInfo> &videoInfo, std::shared_ptr<VideoFrameInfo> &frameInfo) = 0;
    virtual void onFrame(Frame *frame) = 0;
    virtual void onEOS() = 0;
    virtual void onError(const Error &err) = 0;
    virtual EventLoop *getEventLoop() = 0;
};

}
}

#endif
