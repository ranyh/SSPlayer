#include "primitive.h"

namespace playos {

Primitive::Primitive(Type type):
        Element(nullptr),
        m_type(type), m_color(1.0f, 1.0f, 1.0f, 1.0f),
        m_model(1.0f), m_buffers(Buffers::rectangle()),
        m_rounded(0.0f)
{
    m_shader = ShaderManager::instance().getShader("primitive");
}

Primitive::~Primitive()
{
}

void Primitive::setGeometry(float x, float y, float width, float height)
{
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;

    m_model = glm::mat4(1.0f);
    m_model = glm::translate(m_model, glm::vec3(x, y, 0.0f));
    m_model = glm::scale(m_model, glm::vec3(width, height, 1.0f));

    Element::setGeometry(x, y, width, height);
}

void Primitive::setColor(int r, int g, int b, int a)
{
    setColor(float(r/255.0), b/255.0, g/255.0, a/255.0);
}

void Primitive::setColor(float r, float g, float b, float a)
{
    m_color = Color(r, g, b, a);
}

void Primitive::setColor(const Color &color)
{
    m_color = color;
}

void Primitive::setTexture(std::shared_ptr<Texture> texture)
{
    m_texture = texture;
}

void Primitive::onUpdate(UIContext *context, float dt)
{
}

void Primitive::onDraw(UIContext *context)
{
    m_shader->use();
    m_shader->setVec4("geometry", glm::vec4(m_x, context->drawableHeight() - m_y - m_height, m_width, m_height));
    m_shader->setVec4("color", m_color);
    m_shader->setFloat("type", m_type);
    m_shader->setFloat("rounded", m_rounded);
    m_shader->setMat4("projection", context->getProjectionMatrix());
    m_shader->setMat4("view", context->getViewMatrix());
    m_shader->setMat4("model", m_model);

    m_buffers->use();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Primitive::setRounded(int r)
{
    m_rounded = r;
}

}
