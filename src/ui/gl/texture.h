#ifndef __PLAYOS_TEXTURE_H__
#define __PLAYOS_TEXTURE_H__

#include <glad/glad.h>


namespace playos {

class Texture {
public:
    Texture(int type = GL_TEXTURE_2D);
    Texture(void *data, int width, int height, int pixelFmt,
            int type = GL_TEXTURE_2D);
    ~Texture();

    void load(void *data, int width, int height);
    void load(void *data, int width, int height, int pixelFmt);
    void use();

    int width() {
        return m_width;
    }

    int height() {
        return m_height;
    }

private:
    int type;
    GLuint id;
    int m_width, m_height;
    int m_pixelFmt;
};

}

#endif
