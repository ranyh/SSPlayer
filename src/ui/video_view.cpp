#include "video_view.h"

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl/shader_manager.h"


namespace playos {

VideoView::VideoView(Element *parent):
        Element(parent),
        m_shader(ShaderManager::instance().defaultShader()),
        m_model(glm::mat4(1.0f)), m_buffers(Buffers::rectangle())
{
    m_shader->use();
    m_shader->setInt("tex", 0);
}

void VideoView::setTexture(std::shared_ptr<Texture> texture)
{
    m_frameTexture = texture;
}

void VideoView::onUpdate(UIContext *context, float dt)
{
    if (m_frameTexture) {
        m_model         = glm::mat4(1.0f);
        float width = this->width();
        float height = this->height();
        float x, y;
        float w, h;

        float scale = std::min(width / m_frameTexture->width(),
                height / m_frameTexture->height());
        w = m_frameTexture->width() * scale;
        h = m_frameTexture->height() * scale;

        x = this->x() + (width - w) / 2;
        y = this->y() + (height - h) / 2;

        m_model = glm::translate(m_model, glm::vec3(x, y, 0.0f));
        m_model = glm::scale(m_model, glm::vec3(w, h, 1.0f));
    }
}

void VideoView::onDraw(UIContext *context)
{
    if (m_frameTexture) {
        m_shader->use();
        m_shader->setMat4("projection", context->getProjectionMatrix());
        m_shader->setMat4("view", context->getViewMatrix());
        m_shader->setMat4("model", m_model);

        glActiveTexture(GL_TEXTURE0);
        m_frameTexture->use();

        m_buffers->use();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    }
}

}