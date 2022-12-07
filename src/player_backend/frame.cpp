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

private:
    DefaultVideoFrame(std::shared_ptr<VideoFrameInfo> info, uint8_t *data, size_t stride, size_t size):
            VideoFrame(info) {
        m_data = new uint8_t[size];
        memcpy(m_data, data, size);

        this->data[0] = m_data;
        this->size[0] = size;
        this->stride[0] = stride;
        this->offset[0] = 0;
    }

private:
    uint8_t *m_data;

    friend class VideoFrame;
};

std::shared_ptr<VideoFrameInfo> VideoFrameInfo::create(int width, int height, PixelFormat format)
{
    auto f = new VideoFrameInfo(width, height, format);

    return std::shared_ptr<VideoFrameInfo>(f);
}

VideoFrame *VideoFrame::create(std::shared_ptr<VideoFrameInfo> info, uint8_t *data, size_t stride, size_t size)
{
    return new DefaultVideoFrame(info, data, stride, size);
}

}
}
