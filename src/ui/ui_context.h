#ifndef __PLAYOS_UI_CONTEXT_H__
#define __PLAYOS_UI_CONTEXT_H__

#include <glm/glm.hpp>
#include "event.h"


namespace playos {

class UIContext {
public:
    UIContext(int width, int height);
    UIContext();

    void onMouseEvent(Event &event);
    void onWindowResize(int width, int height);

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

protected:
    virtual glm::mat4 createProjectionMatrix();
    virtual glm::mat4 createViewMatrix();
    virtual glm::mat4 createMatrix();

private:
    int m_width, m_height;

    glm::mat4 m_projMatrix;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_matrix;

};

}

#endif
