#ifndef __PLAYOS_BUFFERS_H__
#define __PLAYOS_BUFFERS_H__

#include <stdlib.h>

namespace playos {

class Buffers {
private:
    static const Buffers *s_rectangle;

public:
    static const Buffers *rectangle();
    static void initPrimary();

public:
    Buffers();
    ~Buffers();

    void loadVBO(float *data, size_t size);
    void loadEBO(unsigned int *data, size_t size);
    void vertexAttribPointer(int id, size_t lineSize, size_t offset, size_t size);

    void use() const;

private:
    unsigned int VBO, VAO, EBO;
};

}

#endif
