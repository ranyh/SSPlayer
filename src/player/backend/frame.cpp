#include "frame.h"


namespace playos {
namespace player {

class DefaultVideoFrame: public VideoFrame {
    DefaultVideoFrame(const DefaultVideoFrame& ) = delete;
    DefaultVideoFrame& operator=(const DefaultVideoFrame&) = delete;

public:
    ~DefaultVideoFrame() {
        if (m_data) {
            delete [] m_data;
        }
    }

    void setData(uint8_t *data, size_t size) {
        m_size = size;

        m_data = new uint8_t[size];
        memcpy(m_data, data, size);
    }

    size_t loadData(void *data, size_t size) override {
        return copyData(data, m_data, size);
    }

    void *data() override {
        return m_data;
    }

    size_t size() override {
        return m_size;
    }

private:
    DefaultVideoFrame():
        m_size(0), m_data(nullptr) {}
    DefaultVideoFrame(uint8_t *data, size_t size) {
        setData(data, size);
    }

private:
    size_t m_size;
    uint8_t *m_data;

    friend class VideoFrame;
};

std::shared_ptr<VideoFrameInfo> VideoFrameInfo::create(int width, int height, PixelFormat format)
{
    auto f = new VideoFrameInfo(width, height, format);

    return std::shared_ptr<VideoFrameInfo>(f);
}

std::shared_ptr<VideoFrame> VideoFrame::create(uint8_t *data, size_t size)
{
    auto f = new DefaultVideoFrame(data, size);

    return std::shared_ptr<VideoFrame>(f);
}


std::shared_ptr<Frame> Frame::create(std::shared_ptr<VideoFrame> frame)
{
    auto f = new Frame(frame);

    return std::shared_ptr<Frame>(f);
}

}
}
