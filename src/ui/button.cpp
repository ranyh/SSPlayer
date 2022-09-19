#include "button.h"

#include "gl/shader_manager.h"


namespace playos {

Button::Button(const std::string &text, Element *parent):
        Element(parent), m_text(this, text), m_bg(Primitive::Rectangle),
        m_state(0, 100), m_activeColor(0.0f, 1.0f, 0.0f, 1.0f),
        m_defaultColor(0.0, 0.81, 0.99, 1.0f), m_defaultTextColor(1.0f, 1.0f, 0.0f, 1.0f),
        m_activeTextColor(1.0f)
{
}

Button::~Button()
{
}


void Button::setColor(const Color& color)
{
    m_defaultColor = color;
}

void Button::setActiveColor(const Color& color)
{
    m_activeColor = color;
}

void Button::setTextColor(const Color& color)
{
    m_defaultTextColor = color;
}

void Button::setActiveTextColor(const Color& color)
{
    m_activeTextColor = color;
}

bool Button::onMouseEvent(MouseEvent &event)
{
    if (event.type == Event::MOUSE_BUTTON) {
        if (event.value == MOUSE_BUTTON_LEFT) {
            if (event.state == KEY_PRESSED) {
                for (auto listener : m_clickListeners) {
                    listener->onClick(this);
                }

                m_state = 1;
            } else if (event.state == KEY_RELEASE) {
                m_state = 0;
            }
        }
    }

    return true;
}

void Button::onUpdate(UIContext *context, float dt)
{
    if (isInvalid()) {
        m_bg.setGeometry(x(), y(), width(), height());
        m_bg.setRounded(8);

        m_text.setAlignment(Text::AlignCenter);
        m_text.setGeometry(x(), y(), width(), height());
        m_text.onUpdate(context, dt);
    }

    m_state.update(dt);

    if (m_state == 1) {
        m_text.setColor(m_activeTextColor);
        m_bg.setColor(m_activeColor);
    } else {
        m_text.setColor(m_defaultTextColor);
        m_bg.setColor(m_defaultColor);
    }

}

void Button::onDraw(UIContext *context)
{
    m_bg.draw(context);
    m_text.draw(context);
}

}
