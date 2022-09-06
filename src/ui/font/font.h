#ifndef __PLAYOS_FONT_H__
#define __PLAYOS_FONT_H__

#include <ft2build.h>
#include FT_FREETYPE_H

#include <memory>
#include <string>

namespace playos {

class WordBuffer {
public:
    ~WordBuffer();
    void *buffer();
    int width();
    int height();
    int bearingX();
    int bearingY();
    int advance();

private:
    WordBuffer(FT_GlyphSlot slot);

    FT_GlyphSlot slot;

    friend class Font;
};

class Font {
public:
    using Buffer = std::shared_ptr<WordBuffer>;

public:
    Font(const std::string &name, const std::string &font);
    ~Font();

    void setFontSize(int size);
    bool isFont() { return !error; }

    Buffer getWord(uint64_t w);

private:
    FT_Library library;
    FT_Face face;
    std::string name;

    bool error;
};

}

#endif
