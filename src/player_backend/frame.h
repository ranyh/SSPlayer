#ifndef __PLAYOS_PLAYER_FRAME_H__
#define __PLAYOS_PLAYER_FRAME_H__

#include <algorithm>
#include <memory>
#include <stdint.h>
#include <string.h>


namespace playos {
namespace player {

enum PixelFormat {
    RGB,
    YUV420P,
    NV12,
    DRM,
};

enum AudioSampleFormat {
    AudioUnkown,
    AudioS16,
};

class Frame {
public:
    enum Type {
        Video,
        Audio,
        Subtitle,
    };

public:
    Type type;
    int64_t duration;

public:
    Frame(Type type): type(type) { }

    Frame(const Frame&) = delete;
    Frame(Frame&&) = delete;
};


class VideoFrameInfo: public std::enable_shared_from_this<VideoFrameInfo> {
public:
    int width, height;
    PixelFormat format;
    PixelFormat swFormat;
    int planars;

    int drmFourcc;

    static std::shared_ptr<VideoFrameInfo> create(int width, int height, PixelFormat format);

private:
    VideoFrameInfo(int width, int height, PixelFormat format):
        width(width), height(height), format(format), planars(1) {}
};

class VideoFrame: public Frame {
public:
    static const int MAX_PLANE = 4;

    enum Mode {
        ReadOnly,
        WriteOnly,
        ReadWrite
    };
public:
    VideoFrame(std::shared_ptr<VideoFrameInfo> info): Frame(Frame::Video),
            planes(0), m_frameInfo(info) {
        for (int i = 0; i < MAX_PLANE; i++) {
            data[i] = NULL;
            size[i] = 0;
            stride[i] = 0;
            offset[i] = 0;
        }
    }

    virtual ~VideoFrame(){ }

    virtual bool map(Mode mode) { return true; };
    virtual bool unmap() { return true; };

    void *data[MAX_PLANE];
    size_t size[MAX_PLANE];
    size_t stride[MAX_PLANE];
    int offset[MAX_PLANE];
    int planes;

    VideoFrameInfo *frameInfo() {
        return m_frameInfo.get();
    }

    static VideoFrame *create(std::shared_ptr<VideoFrameInfo> info, uint8_t *data, size_t stride, size_t size);

private:
    VideoFrame(const VideoFrame&) = delete;
    VideoFrame(VideoFrame&&) = delete;

protected:
    std::shared_ptr<VideoFrameInfo> m_frameInfo;
};

class AudioFrame: public Frame {
public:
    static const int MAX_PLANE = 4;

    void *data[MAX_PLANE];
    int samples;
    int channels;
    int freq;
    int planes;
    AudioSampleFormat format;

public:
    AudioFrame(): Frame(Frame::Audio) { }

private:
    AudioFrame(const AudioFrame&) = delete;
    AudioFrame(AudioFrame&&) = delete;
};

}
}

#endif
