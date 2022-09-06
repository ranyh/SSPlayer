#ifndef __PLAYOS_TEXT_H__
#define __PLAYOS_TEXT_H__

#include "element.h"

#include <string>
#include <vector>

#include "font/font_manager.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/buffers.h"


namespace playos {

class Text: public Element {
public:
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

        friend class Text;
    };

    using Char = std::unique_ptr<_char>;

public:
    Text(Element *parent, const std::string &text = "");

    void setText(const std::string &text);
    std::string text();
    void setColor(glm::vec4 color);
    void setFontSize(int size);
    int getFontSize();

    std::shared_ptr<Font> font();
    void setFont(std::shared_ptr<Font> font);

public:
    void onEvent(Event &event) override;
    void onUpdate(UIContext *context, float dt) override;
    void onDraw(UIContext *context) override;

protected:
    virtual std::vector<Char> createChars();
    Char createChar(const Font::Buffer &buffer);

private:
    std::string m_text;
    glm::vec4 m_color;
    int m_fontSize;
    std::shared_ptr<Font> m_font;

    std::shared_ptr<Shader> m_shader;
    const Buffers * const buffers;
    std::vector<Char> m_chars;

    glm::mat4 m_model;
};

}

#endif
