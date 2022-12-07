#include "video_view.h"

#include <stb_image.h>
#include <SDL_syswm.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl/gl_checker.h"
#include "gl/shader_manager.h"
#include "spdlog/spdlog.h"

#include "utils/time.h"

#ifdef __APPLE__
#undef GL_LUMINANCE
#define GL_LUMINANCE GL_RED
#endif


namespace playos {

#ifndef __APPLE__
EGLImage VideoView::FrameRes::createEGLImage(player::VideoFrame *frame)
{
    EGLAttrib const attribute_list[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_LINUX_DRM_FOURCC_EXT, frame->frameInfo()->drmFourcc,
        EGL_DMA_BUF_PLANE0_FD_EXT, *((int *)frame->data[0]),
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, frame->offset[0],
        EGL_DMA_BUF_PLANE0_PITCH_EXT, EGLAttrib(frame->stride[0]),
        EGL_DMA_BUF_PLANE1_FD_EXT, *((int *)frame->data[1]),
        EGL_DMA_BUF_PLANE1_OFFSET_EXT, frame->offset[1],
        EGL_DMA_BUF_PLANE1_PITCH_EXT, EGLAttrib(frame->stride[1]),
        EGL_NONE
    };
    
    EGLImage image = eglCreateImage(m_resourceContext->getEGLDisplay(), EGL_NO_CONTEXT,
            EGL_LINUX_DMA_BUF_EXT, (EGLClientBuffer)NULL, attribute_list);
    if (image == EGL_NO_IMAGE) {
        spdlog::error("createEGLImage error: {}", eglGetError());
    }

    return image;
}
#endif

void VideoView::FrameRes::initTextures(const std::shared_ptr<player::VideoFrameInfo> &frameInfo)
{
    
    frameInfoChanged = frameInfo->width != width ||
                frameInfo->height != height ||
                frameInfo->format != format ||
                frameInfo->swFormat != swFormat;
    if (!frameInfoChanged) {
        return;
    }

    width = frameInfo->width;
    height = frameInfo->height;
    format = frameInfo->format;
    swFormat = frameInfo->swFormat;

    if (!m_textures.empty()) {
        glDeleteTextures(m_textures.size(), m_textures.data());
        m_textures.clear();
    }

    if (format == player::DRM) {
        GLuint id;
        glGenTextures(1, &id);
        GL_CHECK_ERROR("glGenTextures");
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, id);
        GL_CHECK_ERROR("glBindTexture");

        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GL_CHECK_ERROR("glTexParameteri");

        m_textures.push_back(id);
    } else {
        for (int i = 0; i < frameInfo->planars; i++) {
            GLuint id;
            glGenTextures(1, &id);
            GL_CHECK_ERROR("glGenTextures");
            glBindTexture(GL_TEXTURE_2D, id);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            if (format == player::YUV420P) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, i == 0 ? width : width/2, i == 0 ? height : height/2, 0,
                        GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
            } else if (format == player::NV12) {
                if (i == 0) {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0,
                        GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
                } else {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width/2, height/2, 0,
                        GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, NULL);

                }
            } else if (format == player::RGB) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                        GL_RGB, GL_UNSIGNED_BYTE, NULL);
            } else {
                spdlog::error("Not support pixelFormat {} yet.", format);
            }

            GL_CHECK_ERROR("glTexImage2D");

            m_textures.push_back(id);
        }
    }

}

void VideoView::FrameRes::updateTexture(GLuint texture, player::VideoFrame *videoFrame, int index)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    GL_CHECK_ERROR("glBindTexture 0");

    if (format == player::YUV420P) {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, videoFrame->stride[index]);
        GL_CHECK_ERROR("glPixelStorei 0");
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                index == 0 ? width : width/2, index == 0 ? height : height/2,
                GL_LUMINANCE, GL_UNSIGNED_BYTE, videoFrame->data[index]);
    } else if (format == player::NV12) {
        if (index == 0) {
            glPixelStorei(GL_UNPACK_ROW_LENGTH, videoFrame->stride[index]);
            GL_CHECK_ERROR("glPixelStorei 0");
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, videoFrame->data[index]);
        } else {
            glPixelStorei(GL_UNPACK_ROW_LENGTH, videoFrame->stride[index]/2);
            GL_CHECK_ERROR("glPixelStorei 0");
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    width/2, height/2, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, videoFrame->data[index]);
        }
    } else if (format == player::RGB) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                GL_RGB, GL_UNSIGNED_BYTE, videoFrame->data[index]);
    }

    GL_CHECK_ERROR("glTexSubImage2D 0");
}

void VideoView::FrameRes::updateTextures(player::VideoFrame *videoFrame)
{
    if (m_textures.empty()) {
        spdlog::error("VideoView::FrameRes::updateTextures m_textures is empty");
        return;
    }

    if (format == player::DRM) {
#ifdef __APPLE__
        spdlog::error("Apple is not supporting drm");
#else
        EGLImage image = createEGLImage(videoFrame);
        GL_CHECK_ERROR("clear");
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_textures[0]);
        GL_CHECK_ERROR("glBindTexture");
        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
        GL_CHECK_ERROR("glEGLImageTargetTexture2DOES");
        eglDestroyImage(m_resourceContext->getEGLDisplay(), image);
#endif
    } else {
        updateTexture(m_textures[0], videoFrame, 0);
        if (m_textures.size() > 1) {
            updateTexture(m_textures[1], videoFrame, 1);
            if (m_textures.size() > 2) {
                updateTexture(m_textures[2], videoFrame, 2);
            }
        }

        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    }
}

std::shared_ptr<Shader> VideoView::FrameRes::useTextures(UIContext *context)
{
    std::shared_ptr<Shader> shader;

    if (format == player::YUV420P) {
        shader = ShaderManager::instance().getShader("yuv");

        shader->use();
        shader->setInt("u_texture0", 0);
        shader->setInt("u_texture1", 1);
        shader->setInt("u_texture2", 2);
        shader->setFloat("u_chroma_div_w", 1.0f);
        shader->setFloat("u_chroma_div_h", 1.0f);

        for (int i = 0; i < m_textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, m_textures[i]);
            GL_CHECK_ERROR("glBindTexture");
        }
    } else if (format == player::RGB) {
        shader = ShaderManager::instance().defaultShader();

        shader->use();
        shader->setInt("tex", 0);
        glActiveTexture(GL_TEXTURE0);
        if (!m_textures.empty())
            glBindTexture(GL_TEXTURE_2D, m_textures[0]);
    } else if (format == player::NV12) {
        shader = ShaderManager::instance().getShader("nv12");

        shader->use();
        shader->setInt("u_texture0", 0);
        shader->setInt("u_texture1", 1);

        for (int i = 0; i < m_textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, m_textures[i]);
            GL_CHECK_ERROR("glBindTexture");
        }
    } else if (format == player::DRM) {
        shader = ShaderManager::instance().getShader("yuv_ext");

        shader->use();
        shader->setInt("u_texture0", 0);
        glActiveTexture(GL_TEXTURE0);
        if (!m_textures.empty())
            glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_textures[0]);
    }

    

    return shader;
}

VideoView::VideoView(ResourceContext *context, Element *parent):
        Element(parent), m_resourceContext(context),
        m_model(glm::mat4(1.0f)),
        m_buffers(Buffers::rectangle()),
        m_pending(context), m_current(context)
{
}

VideoView::~VideoView()
{
}

void VideoView::prepareTextures(const std::shared_ptr<player::VideoFrameInfo> &frameInfo)
{
    m_current.initTextures(frameInfo);
    m_pending.initTextures(frameInfo);
}

void VideoView::load(player::VideoFrame *frame)
{
    // MEASURE_BEGIN(a);
    m_pending.updateTextures(frame);
    // MEASURE_END("updateTextures time: ", a);

    swap(m_current, m_pending);
}

void VideoView::onUpdate(UIContext *context, float dt)
{
    if (m_current.frameInfoChanged) {
        m_current.frameInfoChanged = false;
        int texWidth = m_current.width;
        int texHeight = m_current.height;

        m_model         = glm::mat4(1.0f);
        float width = this->width();
        float height = this->height();
        float x, y;
        float w, h;

        float scale = std::min(width / texWidth, height / texHeight);
        w = texWidth * scale;
        h = texHeight * scale;

        x = this->x() + (width - w) / 2;
        y = this->y() + (height - h) / 2;

        m_model = glm::translate(m_model, glm::vec3(x, y, 0.0f));
        m_model = glm::scale(m_model, glm::vec3(w, h, 1.0f));
    }
}

void VideoView::onDraw(UIContext *context)
{
    if (m_current.width == 0 || m_current.height == 0) {
        return;
    }

    auto shader = m_current.useTextures(context);
    if (shader) {
        shader->setMat4("projection", context->getProjectionMatrix());
        shader->setMat4("view", context->getViewMatrix());
        shader->setMat4("model", m_model);

        m_buffers->use();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}

}