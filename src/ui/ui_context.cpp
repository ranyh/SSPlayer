#include "ui_context.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl/buffers.h"
#include "gl/shader_manager.h"
#include "icon.h"
#include "font/font_manager.h"


namespace playos {

UIContext::UIContext(const std::string &resourceDir, int width, int height):
        m_width(width), m_height(height),
        m_projMatrix(createProjectionMatrix()),
        m_viewMatrix(createViewMatrix()),
        m_matrix(createMatrix()),
        m_resourceContextCreator(nullptr)
{
    Buffers::initPrimary();

    FontManager::instance().addFont("default", resourceDir + "/font/FiraCode-Regular.ttf");
    FontManager::instance().addFont("mdi", resourceDir + "/font/materialdesignicons-webfont.ttf");

    ShaderManager::instance().init(resourceDir);
    Icon::init(resourceDir);
}

UIContext::UIContext(const std::string &resourceDir):
        UIContext(resourceDir, 640, 480)
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

void UIContext::scissor(int x, int y, int width, int height)
{
    glScissor(x, m_height-y-height, width, height);
}

void UIContext::resetScissor()
{
    glScissor(0, 0, m_width, m_height);
}

glm::mat4 UIContext::createProjectionMatrix()
{
    // m_projection = glm::perspective(glm::radians(45.0f), (float)parent->width() / (float)parent->height(), 0.1f, 10000.0f);

    // return glm::ortho(-float(m_width), float(m_width),
    //                 -float(m_height), float(m_height), -1.0f, 1.0f);
    return glm::ortho(0.0f, float(m_width), float(m_height), 0.0f, -1.0f, 1.0f);
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
