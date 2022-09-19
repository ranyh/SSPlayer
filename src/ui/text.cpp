#include "text.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assert.h>
#include <algorithm>

#include "gl/shader_manager.h"


namespace playos {

Text::Text(Element *parent, const std::string &text):
        Element(parent), m_text(text), m_color(0.0f, 0.0f, 0.0f, 1.0f),
        buffers(Buffers::rectangle()), m_fontSize(15),
        m_alignment(AlignVCenter | AlignLeft),
        m_forceVCenter(false)
{
    m_shader = ShaderManager::instance().getShader("font");
    m_shader->use();
    m_shader->setVec4("color", m_color);
    m_shader->setInt("tex", 0);
}

void Text::setText(const std::string &text)
{
    if (m_text != text) {
        m_text = text;
        invalid();
    }
}

std::string Text::text()
{
    return m_text;
}

void Text::setColor(Color color)
{
    m_color = color;
}

void Text::setFontSize(int size)
{
    assert(size > 0);
    m_fontSize = size;
    if (height() == 0) {
        setSize(0, size);
    }

    invalid();
}

int Text::getFontSize()
{
    return m_fontSize;
}

void Text::setAlignment(int align)
{
    int h = align & 0x07;
    int v = align & 0x38;

    switch (h) {
    case 0:
        align |= (m_alignment & 0x07);
        break;
    case AlignLeft:
    case AlignRight:
    case AlignHCenter:
        break;
    default:
        printf("Text::setAlignment unexpected horizontal alginment: %x\n", align);
        return;
    }

    switch (v) {
    case 0:
        align |= (m_alignment & 0x38);
        break;
    case AlignTop:
    case AlignBottom:
    case AlignVCenter:
        break;
    default:
        printf("Text::setAlignment unexpected vertical alginment: %x\n", align);
        return;
    }

    m_alignment = align;
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

int Text::textWidth()
{
    return m_textWidth;
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
        calculateTextWidth();

        float x = this->x();
        float mH = 0.0;
        for (int i = 0; i < m_chars.size(); i++) {
            m_chars[i]->w = m_chars[i]->width;
            m_chars[i]->h = m_chars[i]->height;
            m_chars[i]->y = y() - m_chars[i]->bearingY;

            mH = std::max(mH, m_chars[i]->h);
            m_chars[i]->x = x + m_chars[i]->bearingX;
            x += (m_chars[i]->advance >> 6);

            switch (m_alignment & 0x07) {
            case AlignLeft:
                break;
            case AlignRight:
                m_chars[i]->x += width() - m_textWidth;
                break;
            case AlignHCenter:
                m_chars[i]->x -= (m_textWidth - width()) / 2.0f;
                break;
            }

        }

        if (m_forceVCenter) {
            for (int i = 0; i < m_chars.size(); i++) {
                m_chars[i]->y = y() + (height() - m_chars[i]->h) / 2;
            }
        } else {
            for (int i = 0; i < m_chars.size(); i++) {
                switch (m_alignment & 0x38) {
                case AlignTop:
                    m_chars[i]->y += std::min(mH, float(height()));
                    break;
                case AlignBottom:
                    m_chars[i]->y += height();
                    break;
                case AlignVCenter:
                    m_chars[i]->y = y() - m_chars[i]->bearingY + (height() + mH) / 2;
                    break;
                }
            }
        }
    }
}

void Text::onDraw(UIContext *context)
{
    buffers->use();
    m_shader->use();
    m_shader->setMat4("projection", context->getProjectionMatrix());
    m_shader->setMat4("view", context->getViewMatrix());
    m_shader->setVec4("color", m_color);

    glActiveTexture(GL_TEXTURE0);

    float x = this->x();
    for (int i = 0; i < m_chars.size(); i++) {
        m_model = glm::mat4(1.0f);

        m_model = glm::translate(m_model, glm::vec3(m_chars[i]->x, m_chars[i]->y, 0.0f));
        m_model = glm::scale(m_model, glm::vec3(m_chars[i]->w, m_chars[i]->h, 1.0f));
        m_shader->setMat4("model", m_model);

        m_chars[i]->use();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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

void Text::setForceVCenter(bool force)
{
    m_forceVCenter = true;
}

void Text::calculateTextWidth()
{
    m_textWidth = 0;

    for (int i = 0; i < m_chars.size(); i++) {
        m_textWidth += (m_chars[i]->advance >> 6);
    }
}

}