#ifndef __GL_CHECKER_H__
#define __GL_CHECKER_H__

#include "glad/glad.h"

#if 1
#define GL_CHECK_ERROR(msg) { \
    GLenum __err; \
    while ((__err = glGetError()) != GL_NO_ERROR) { \
        const char *eMsg = NULL; \
        switch (__err) { \
        case GL_INVALID_ENUM: \
            eMsg = "INVALID_ENUM"; \
            break; \
        case GL_INVALID_VALUE: \
            eMsg = "INVALID_VALUE"; \
            break; \
        case GL_INVALID_OPERATION: \
            eMsg = "INVALID_OPERATION"; \
            break; \
        case GL_STACK_OVERFLOW: \
            eMsg = "STACK_OVERFLOW"; \
            break; \
        case GL_STACK_UNDERFLOW: \
            eMsg = "STACK_UNDERFLOW"; \
            break; \
        case GL_OUT_OF_MEMORY: \
            eMsg = "OUT_OF_MEMORY"; \
            break; \
        case GL_INVALID_FRAMEBUFFER_OPERATION: \
            eMsg = "INVALID_FRAMEBUFFER_OPERATION"; \
            break; \
        } \
        spdlog::error("{}:{}, " msg ": {}", __FILE__, __LINE__, eMsg); \
    } \
}
#else
#define GL_CHECK_ERROR(msg)
#endif

#endif
