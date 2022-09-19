#ifndef __PLAYOS_TEXT_H__
#define __PLAYOS_TEXT_H__

#include "element.h"

#include <string>
#include <vector>

#include "color.h"
#include "font/font_manager.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/buffers.h"


namespace playos {

class Text: public Element {
public:
    enum Alignment {
        AlignLeft       = 1 << 0,
        AlignRight      = 1 << 1,
        AlignHCenter    = 1 << 2,

        AlignTop        = 1 << 3,
        AlignBottom     = 1 << 4,
        AlignVCenter    = 1 << 5,

        AlignCenter     = AlignHCenter | AlignVCenter,
    };

    class _char {
    public:
        void use() {
            texture->use();
        }

    private:
        std::unique_ptr<Texture> texture;

        int width;
        int height;
        int bearingX;
        int bearingY;
        int advance;

        float x, y, w, h;

        friend class Text;
    };

    using Char = std::unique_ptr<_char>;

public:
    Text(Element *parent, const std::string &text = "");

    void setText(const std::string &text);
    std::string text();
    void setColor(Color color);
    void setFontSize(int size);
    int getFontSize();
    void setAlignment(int align);

    std::shared_ptr<Font> font();
    void setFont(std::shared_ptr<Font> font);

    int textWidth();

public:
    void onEvent(Event &event) override;
    void onUpdate(UIContext *context, float dt) override;
    void onDraw(UIContext *context) override;

protected:
    virtual std::vector<Char> createChars();
    Char createChar(const Font::Buffer &buffer);
    void setForceVCenter(bool force);

private:
    void calculateTextWidth();

private:
    std::string m_text;
    Color m_color;
    int m_fontSize;
    int m_alignment;
    int m_textWidth;
    bool m_forceVCenter;
    std::shared_ptr<Font> m_font;

    std::shared_ptr<Shader> m_shader;
    const Buffers * const buffers;
    std::vector<Char> m_chars;

    glm::mat4 m_model;
};

}

#endif
