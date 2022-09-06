#include "text.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assert.h>

#include "gl/shader_manager.h"


namespace playos {

Text::Text(Element *parent, const std::string &text):
        Element(parent), m_text(text), m_color(0.0f, 0.0f, 0.0f, 1.0f),
        buffers(Buffers::rectangle()), m_fontSize(16)
{
    m_shader = ShaderManager::instance().getShader("font");
    m_shader->use();
    m_shader->setVec4("color", m_color);
    m_shader->setInt("tex", 0);
}

void Text::setText(const std::string &text)
{
    m_text = text;
    invalid();
}

std::string Text::text()
{
    return m_text;
}

void Text::setColor(glm::vec4 color)
{
    m_color = color;
    m_shader->use();
    m_shader->setVec4("color", m_color);
}

void Text::setFontSize(int size)
{
    assert(size > 0);
    m_fontSize = size;
    invalid();
}

int Text::getFontSize()
{
    return m_fontSize;
}


std::shared_ptr<Font> Text::font()
{
    return m_font;
}

void Text::setFont(std::shared_ptr<Font> font)
{
    m_font = font;
    invalid();
}


void Text::onEvent(Event &event)
{
}

void Text::onUpdate(UIContext *context, float dt)
{
    if (!m_font) {
        m_font = FontManager::instance().getFont("default");
    }

    if (isInvalid()) {
        m_font->setFontSize(m_fontSize);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        m_chars.clear();
        m_chars = createChars();
    }
}

void Text::onDraw(UIContext *context)
{
    buffers->use();
    m_shader->use();
    m_shader->setMat4("projection", context->getProjectionMatrix());
    m_shader->setMat4("view", context->getViewMatrix());

    glActiveTexture(GL_TEXTURE0);

    float x = this->x();
    for (int i = 0; i < m_chars.size(); i++) {
        m_model = glm::mat4(1.0f);

        m_chars[i]->use();

        float s = 0.96f;
        float w = s * m_chars[i]->width;
        float h = s * m_chars[i]->height;
        m_model = glm::translate(m_model,
                glm::vec3(
                    x + m_chars[i]->bearingX * s + w,
                    y() - (m_chars[i]->height - m_chars[i]->bearingY) * s + h,
                    0.0f));

        m_model = glm::scale(m_model, glm::vec3(w, h, 1.0f));

        m_shader->setMat4("model", m_model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        x += (m_chars[i]->advance >> 6) + w;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

std::vector<Text::Char> Text::createChars()
{
    std::vector<Char> textures;

    for (int i = 0; i < m_text.length(); i++) {
        auto buffer = m_font->getWord(m_text[i]);

        if (!buffer) {
            continue;
        }
        textures.push_back(std::move(createChar(buffer)));
    }

    return textures;
}

Text::Char Text::createChar(const Font::Buffer &buffer)
{
    auto ch = new _char();

    ch->texture = std::unique_ptr<Texture>(
            new Texture(
                buffer->buffer(),
                buffer->width(),
                buffer->height(),
                GL_RED));
    ch->width = buffer->width();
    ch->height = buffer->height();
    ch->bearingX = buffer->bearingX();
    ch->bearingY = buffer->bearingY();
    ch->advance = buffer->advance();

    return Char(ch);
}

}