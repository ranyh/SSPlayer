#ifndef __PLAYOS_Video_VIEW_H__
#define __PLAYOS_Video_VIEW_H__

#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <queue>

#include "element.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/buffers.h"

#include "frame.h"


namespace playos {

class VideoView: public Element {
public:
    class FrameRes {
    public:
        int width;
        int height;
        int format;
        int swFormat;
        bool frameInfoChanged;

        std::vector<GLuint> m_textures;
        ResourceContext *m_resourceContext;

        FrameRes(ResourceContext *resourceContext):
                width(0), height(0), format(0), swFormat(0),
                frameInfoChanged(0),
                m_resourceContext(resourceContext) {
        }


        FrameRes(FrameRes const&) = delete;
        FrameRes& operator = (FrameRes const&) = delete;

        friend void swap(VideoView::FrameRes& lhs, VideoView::FrameRes& rhs) {
            std::swap(lhs.m_textures, rhs.m_textures);

            std::swap(lhs.width, rhs.width);
            std::swap(lhs.height, rhs.height);
            std::swap(lhs.format, rhs.format);
            std::swap(lhs.swFormat, rhs.swFormat);
        }

        ~FrameRes() {
            if (!m_textures.empty()) {
                glDeleteTextures(m_textures.size(), m_textures.data());
                m_textures.clear();
            }
        }

        void initTextures(const std::shared_ptr<player::VideoFrameInfo> &frameInfo);
        void updateTexture(GLuint texture, player::VideoFrame *videoFrame, int index);
        void updateTextures(player::VideoFrame *videoFrame);
        std::shared_ptr<Shader> useTextures(UIContext *context);
#ifndef __APPLE__
        EGLImage createEGLImage(player::VideoFrame *frame);
#endif
    };

public:
    VideoView(ResourceContext *context, Element *parent);
    ~VideoView();

    void prepareTextures(const std::shared_ptr<player::VideoFrameInfo> &frameInfo);
    void load(player::VideoFrame *frame);

    void resThreadFunc();

protected:
    void onUpdate(UIContext *context, float dt) override;
    void onDraw(UIContext *context) override;

private:
    std::unique_ptr<ResourceContext> m_resourceContext;
    const Buffers * const m_buffers;
    glm::mat4 m_model;

    FrameRes m_current;
    FrameRes m_pending;
};

}

#endif
