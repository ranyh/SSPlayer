#include "layer.h"

#include <assert.h>


namespace playos {

Group::Group(Element *parent):
    Element(parent), m_activeElement(nullptr)
{
}

void Group::onEvent(Event &event)
{
    if (event.type == Event::MOUSE_BUTTON ||
            event.type == Event::MOUSE_MOVE ||
            event.type == Event::MOUSE_WHEEL) {
        onMouseEvent(event);
        return;
    }

    for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
        (*it)->onEvent(event);
    }
}

bool Group::onMouseEvent(MouseEvent &event)
{
    if (event.type == Event::MOUSE_BUTTON && event.state == KEY_RELEASE) {
        if (m_activeElement) {
            m_activeElement->onMouseEvent(event);
        }
        return true;
    }

    if (event.type == Event::MOUSE_BUTTON ||
                event.type == Event::MOUSE_MOVE) {
        for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
            if ((*it)->checkPosition(event.x, event.y)) {
                if (m_activeElement != (*it)) {
                    MouseEvent ev = event;
                    ev.type = Event::MOUSE_LEAVE;
                    if (m_activeElement != nullptr) {
                        m_activeElement->onMouseEvent(ev);
                    }

                    ev.type = Event::MOUSE_ENTER;
                    (*it)->onMouseEvent(ev);
                    m_activeElement = (*it);
                }

                (*it)->onMouseEvent(event);
                return true;
            }
        }

        if (event.type == Event::MOUSE_MOVE && m_activeElement) {
            MouseEvent ev = event;
            ev.type = Event::MOUSE_LEAVE;
            m_activeElement->onMouseEvent(ev);
            m_activeElement = nullptr;
        }
    } else if (event.type == Event::MOUSE_WHEEL) {
        if (m_activeElement) {
            m_activeElement->onMouseEvent(event);
        }
    }

    return false;
}

void Group::onUpdate(UIContext *context, float dt)
{
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
        (*it)->update(context, dt);
    }
}

void Group::onDraw(UIContext *context)
{
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
        (*it)->draw(context);
    }
}

void Group::add(Element *el) {
    assert(el);
    el->setParent(this);
    m_elements.push_back(el);
}

void Group::remove(Element *el) {
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
        if (*it == el) {
            el->setParent(nullptr);
            m_elements.erase(it);
            break;
        }
    }
}

void Group::invalid()
{
    Element::invalid();

    for (auto it : m_elements) {
        it->invalid();
    }
}


}