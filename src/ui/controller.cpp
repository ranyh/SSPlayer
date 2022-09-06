#include "controller.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <unordered_map>

#include "gl/shader_manager.h"


namespace playos {

Controller::Controller(Element *parent):
        Element(parent), buffers(Buffers::rectangle()),
        m_model(glm::mat4(1.0f)), text(parent)
{
    setSize(parent->width(), 32);

    m_shader = ShaderManager::instance().getShader("color");
    m_shader->use();
    m_shader->setVec4("color", glm::vec4(0.0f, 1.0f, 0.0f, 0.8f));

    text.setText("No video");
    text.setColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    text.setFontSize(18);

    icon.emplace_back(parent, "mdi-alpha-i-circle-outline");
    icon.emplace_back(parent, "mdi-folder-lock");
    icon.emplace_back(parent, "mdi-youtube-gaming");
    icon.emplace_back(parent, "mdi-window-closed");
    icon.emplace_back(parent, "mdi-window-closed-variant");
    icon.emplace_back(parent, "mdi-window-maximize");
    icon.emplace_back(parent, "mdi-window-minimize");
    icon.emplace_back(parent, "mdi-window-open");

    for (auto &i : icon) {
        i.setColor(glm::vec4(1.0f, 0.6f, 0.0f, 1.0f));
    }
}

void Controller::onEvent(Event &event)
{
    if (event.type == Event::WINDOW_RESIZE) {
        setSize(parent()->width(), height());
    }
}

void Controller::onUpdate(UIContext *context, float dt)
{
    if (isInvalid()) {
        float width = this->width();
        float height = this->height();
        m_model        = glm::mat4(1.0f);

        setPosition(0.0f, height-parent()->height());

        m_model = glm::translate(m_model, glm::vec3(x(), y(), 0.0f));

        // m_model = glm::translate(m_model, glm::vec3(0.5f * width, 0.5f * height, 0.0f));
        m_model = glm::rotate(m_model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // m_model = glm::translate(m_model, glm::vec3(-0.5f * width, -0.5f * height, 0.0f));

        m_model = glm::scale(m_model, glm::vec3(width, height, 1.0f));

        int size = 24;
        int _x = x() - (icon.size() + 1) * size, _y = size - parent()->height();
        for (int i = 0; i < icon.size(); ++i) {
            auto &_icon = icon[i];
            _icon.setSize(size);
            _icon.setPosition(_x, _y);
            _x = _x + 32 * 2;
        }

        text.setPosition(-65, -80);
    }

    text.update(context, dt);

    for (auto &i : icon) {
        i.update(context, dt);
    }
}

void Controller::onDraw(UIContext *context)
{
    m_shader->use();
    m_shader->setMat4("projection", context->getProjectionMatrix());
    m_shader->setMat4("view", context->getViewMatrix());
    m_shader->setMat4("model", m_model);

    buffers->use();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    text.draw(context);
    for (auto &i : icon) {
        i.draw(context);
    }
}

}
