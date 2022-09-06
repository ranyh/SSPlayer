#include "font.h"

#include <iostream>


namespace playos {

static const char *encodingToStr(int e)
{
    switch (e) {
    case ft_encoding_none:
        return "FT_ENCODING_NONE";
    case ft_encoding_unicode:
        return "FT_ENCODING_UNICODE";
    case ft_encoding_symbol:
        return "FT_ENCODING_MS_SYMBOL";
    case ft_encoding_latin_1:
        return "FT_ENCODING_ADOBE_LATIN_1";
    case ft_encoding_latin_2:
        return "FT_ENCODING_OLD_LATIN_2";
    case ft_encoding_sjis:
        return "FT_ENCODING_SJIS";
    case ft_encoding_gb2312:
        return "FT_ENCODING_PRC";
    case ft_encoding_big5:
        return "FT_ENCODING_BIG5";
    case ft_encoding_wansung:
        return "FT_ENCODING_WANSUNG";
    case ft_encoding_johab:
        return "FT_ENCODING_JOHAB";
    case ft_encoding_adobe_standard:
        return "FT_ENCODING_ADOBE_STANDARD";
    case ft_encoding_adobe_expert:
        return "FT_ENCODING_ADOBE_EXPERT";
    case ft_encoding_adobe_custom:
        return "FT_ENCODING_ADOBE_CUSTOM";
    case ft_encoding_apple_roman:
        return "FT_ENCODING_APPLE_ROMAN";
    }

    return "";
}

WordBuffer::WordBuffer(FT_GlyphSlot slot):
        slot(slot)
{
}

WordBuffer::~WordBuffer()
{
    
}

void *WordBuffer::buffer()
{
    return slot->bitmap.buffer;
}

int WordBuffer::width()
{
    return slot->bitmap.width;
}

int WordBuffer::height()
{
    return slot->bitmap.rows;
}

int WordBuffer::bearingX()
{
    return slot->bitmap_left;
}

int WordBuffer::bearingY()
{
    return slot->bitmap_top;
}

int WordBuffer::advance()
{
    return slot->advance.x;
}

Font::Font(const std::string &name, const std::string &font):
        error(false)
{
    if (FT_Init_FreeType(&library) != FT_Err_Ok) {
        std::cerr << "Freetype init error." << std::endl;
        error = true;
        return;
    }

    if (FT_New_Face(library, font.c_str(), 0, &face) != FT_Err_Ok) {
        std::cerr << "Load font error." << std::endl;
        error = true;
        return;
    }

    FT_Select_Charmap(face , FT_ENCODING_UNICODE);

    setFontSize(16);
}

Font::~Font()
{
    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

void Font::setFontSize(int size)
{
    FT_Set_Pixel_Sizes(face, 0, size);
}

Font::Buffer Font::getWord(uint64_t w)
{
    int ret = FT_Load_Char(face, w, FT_LOAD_RENDER);
    if (ret != 0) {
        return nullptr;
    }

    return Font::Buffer(new WordBuffer(face->glyph));
}

}
