#include "texture.h"

#include <stdio.h>
namespace playos {

Texture::Texture(int type): type(type), id(0)
{
    if (type == GL_TEXTURE_2D) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        // set the texture wrapping parameters
        // set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

Texture::Texture(void *data, int width, int height, int pixelFmt, int type):
        Texture(type)
{
    load(data, width, height, pixelFmt);
}

Texture::~Texture()
{
    if (id != 0) {
        glDeleteTextures(1, &id);
    }
}

void Texture::load(void *data, int width, int height)
{
    load(data, width, height, GL_RGB);
}

void Texture::load(void *data, int width, int height, int pixelFmt)
{
    use();

    glTexImage2D(type, 0, pixelFmt, width, height, 0,
            pixelFmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(type);
    
    glBindTexture(GL_TEXTURE_2D, 0);

    m_width = width;
    m_height = height;
}

void Texture::use()
{
    glBindTexture(type, id);
}


}
