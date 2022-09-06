#include "ui_context.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl/buffers.h"
#include "gl/shader_manager.h"


namespace playos {

UIContext::UIContext(int width, int height):
        m_width(width), m_height(height),
        m_projMatrix(createProjectionMatrix()),
        m_viewMatrix(createViewMatrix()),
        m_matrix(createMatrix())
{
    Buffers::initPrimary();
    ShaderManager::instance().init();
}

UIContext::UIContext():
        UIContext(640, 480)
{
}

void UIContext::onMouseEvent(Event &event)
{

}

void UIContext::onWindowResize(int width, int height)
{
    m_width = width;
    m_height = height;

    m_projMatrix = createProjectionMatrix();
    m_viewMatrix = createViewMatrix();
    m_matrix = createMatrix();
}

glm::mat4 UIContext::createProjectionMatrix()
{
    // m_projection = glm::perspective(glm::radians(45.0f), (float)parent->width() / (float)parent->height(), 0.1f, 10000.0f);

    return glm::ortho(-float(m_width), float(m_width),
                    -float(m_height), float(m_height), -1.0f, 1.0f);
    // return glm::ortho(0.0f, float(m_width), 0.0f, float(m_height), -1.0f, 1.0f);
}

glm::mat4 UIContext::createViewMatrix()
{
    return glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f));
}

glm::mat4 UIContext::createMatrix()
{
    return m_projMatrix * m_viewMatrix;
}

}
