#include "icon_button.h"

#include "gl/shader_manager.h"


namespace playos {

IconButton::IconButton(const std::string &text, Element *parent):
        Element(parent), m_icon(this, text), m_bg(Primitive::Rectangle),
        m_state(0, 100), m_activeColor(0.4f, 0.4f, 0.4f, 1.0f),
        m_defaultColor(0.0f, 0.0, 0.0, 0.0f), m_defaultIconColor(1.0f, 1.0f, 1.0f, 1.0f),
        m_activeIconColor(0.0f, 0.81, 0.99, 1.0f)
{
    m_icon.setSize(16);
}

IconButton::~IconButton()
{
}


void IconButton::setColor(Color color)
{
    m_defaultColor = color;
}

void IconButton::setActiveColor(Color color)
{
    m_activeColor = color;
}

void IconButton::setIconColor(Color color)
{
    m_defaultIconColor = color;
}

void IconButton::setActiveIconColor(Color color)
{
    m_activeIconColor = color;
}

void IconButton::setIcon(const std::string &icon)
{
    m_icon.setText(icon);
    invalid();
}

bool IconButton::onMouseEvent(MouseEvent &event)
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

void IconButton::onUpdate(UIContext *context, float dt)
{
    if (isInvalid()) {
        m_bg.setGeometry(x(), y(), width(), height());
        m_bg.setRounded(8);

        m_icon.setAlignment(Icon::AlignCenter);
        m_icon.setGeometry(x(), y(), width(), height());
        m_icon.onUpdate(context, dt);
    }

    m_state.update(dt);

    if (m_state == 1) {
        m_icon.setColor(m_activeIconColor);
        m_bg.setColor(m_activeColor);
    } else {
        m_icon.setColor(m_defaultIconColor);
        m_bg.setColor(m_defaultColor);
    }

}

void IconButton::onDraw(UIContext *context)
{
    m_bg.draw(context);
    m_icon.draw(context);
}

}
