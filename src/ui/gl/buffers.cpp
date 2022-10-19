#include "buffers.h"

#include <glad/glad.h>

#include <stdio.h>


namespace playos {

static float vertices[] = {
    // positions        // texture coords
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // bottom right
     1.0f,  0.0f, 0.0f, 1.0f, 0.0f, // bottom left
     0.0f,  0.0f, 0.0f, 0.0f, 0.0f, // top left
     0.0f,  1.0f, 0.0f, 0.0f, 1.0f  // top right
};

static unsigned int rectIndices[] = {
    0, 3, 1, // first triangle
    3, 2, 1  // second triangle
};

const Buffers * Buffers::s_rectangle = nullptr;

Buffers::Buffers(): EBO(0)
{
    if (glGenVertexArrays)
        glGenVertexArrays(1, &VAO);
    else if (glGenVertexArraysOES)
        glGenVertexArraysOES(1, &VAO);
    else {
        printf("Not support vertex array object\n");
        exit(-1);
    }


    glGenBuffers(1, &VBO);
}

Buffers::~Buffers()
{
    if (EBO != 0)
        glDeleteBuffers(1, &EBO);

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void Buffers::loadVBO(float *data, size_t size)
{
    if (glBindVertexArray) {
        glBindVertexArray(VAO);
    } else {
        glBindVertexArrayOES(VAO);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

void Buffers::loadEBO(unsigned int *data, size_t size)
{
    if (EBO != 0)
        glDeleteBuffers(1, &EBO);

    if (glBindVertexArray) {
        glBindVertexArray(VAO);
    } else {
        glBindVertexArrayOES(VAO);
    }

    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

void Buffers::vertexAttribPointer(int id, size_t lineSize, size_t offset, size_t size)
{
    if (glBindVertexArray) {
        glBindVertexArray(VAO);
    } else {
        glBindVertexArrayOES(VAO);
    }

    glVertexAttribPointer(id, size, GL_FLOAT, GL_FALSE,
            lineSize * sizeof(float), (void*)(offset * sizeof(float)));
    glEnableVertexAttribArray(id);
}

void Buffers::use() const
{
    if (glBindVertexArray) {
        glBindVertexArray(VAO);
    } else {
        glBindVertexArrayOES(VAO);
    }

    if (EBO != 0)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
}

const Buffers *Buffers::rectangle()
{
    return s_rectangle;
}

void Buffers::initPrimary()
{
    s_rectangle = new Buffers();
    auto rect = const_cast<Buffers *>(s_rectangle);

    rect->loadVBO(vertices, sizeof(vertices));
    rect->loadEBO(rectIndices, sizeof(rectIndices));

    rect->vertexAttribPointer(0, 5, 0, 3);
    rect->vertexAttribPointer(1, 5, 3, 2);
}

}
