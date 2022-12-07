#ifndef __PLAYOS_UI_CONTEXT_H__
#define __PLAYOS_UI_CONTEXT_H__

#include <glm/glm.hpp>
#include "event.h"

#ifdef __APPLE__
typedef void * EGLDisplay;
#else
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif

#include <string>


namespace playos {

class ResourceContext {
public:
    virtual ~ResourceContext() {}
    virtual int makeCurrent() = 0;
    virtual int clearCurrent() = 0;
    virtual EGLDisplay getEGLDisplay() = 0;
};

class ResourceContextCreator {
public:
    virtual ResourceContext *createResourceContext() = 0;
};

class UIContext {
public:
    UIContext(const std::string &resourceDir, int width, int height);
    UIContext(const std::string &resourceDir);

    void onMouseEvent(Event &event);
    void onWindowResize(int width, int height);
    void scissor(int x, int y, int width, int height);
    void resetScissor();



    glm::mat4 getProjectionMatrix() {
        return m_projMatrix;
    }

    glm::mat4 getViewMatrix() {
        return m_viewMatrix;
    }

    glm::mat4 getMatrix() {
        return m_matrix;
    }

    int drawableWidth() {
        return m_width;
    }

    int drawableHeight() {
        return m_height;
    }

    void setResourceContextCreator(ResourceContextCreator *creator) {
        m_resourceContextCreator = creator;
    }

    ResourceContext *createResourceContext() {
        if (m_resourceContextCreator) {
            return m_resourceContextCreator->createResourceContext();
        }

        return nullptr;
    }

protected:
    virtual glm::mat4 createProjectionMatrix();
    virtual glm::mat4 createViewMatrix();
    virtual glm::mat4 createMatrix();

private:
    int m_width, m_height;

    glm::mat4 m_projMatrix;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_matrix;

    ResourceContextCreator *m_resourceContextCreator;
};

}

#endif
