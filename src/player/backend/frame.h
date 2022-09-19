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
    RGBA,
    BRG,
};

struct VideoFrameInfo: public std::enable_shared_from_this<VideoFrameInfo> {
    int width, height;
    PixelFormat format;

    static std::shared_ptr<VideoFrameInfo> create(int width, int height, PixelFormat format);

private:
    VideoFrameInfo() {}
    VideoFrameInfo(int width, int height, PixelFormat format):
        width(width), height(height), format(format) {}
};

struct VideoFrame: public std::enable_shared_from_this<VideoFrame> {
public:
    enum Mode {
        ReadOnly,
        WriteOnly,
        ReadWrite
    };
public:
    virtual ~VideoFrame(){ }

    virtual bool map(Mode mode) { return true; };
    virtual bool unmap() { return true; };
    virtual size_t loadData(void *data, size_t size) = 0;
    virtual void *data() = 0;
    virtual size_t size() = 0;

   static std::shared_ptr<VideoFrame> create(uint8_t *data, size_t size);

protected:
    size_t copyData(void *dest, void *src, size_t size) {
        size = std::min(size, this->size());
        memcpy(dest, src, size);

        return size;
    }
};

struct Frame: public std::enable_shared_from_this<Frame> {
public:
    std::shared_ptr<VideoFrame> vFrame;
    int64_t current;

public:
    static std::shared_ptr<Frame> create(std::shared_ptr<VideoFrame> frame);
    ~Frame() { }

private:
    Frame(const std::shared_ptr<VideoFrame> frame):
        vFrame(frame), current(0) { }
};

}
}

#endif
