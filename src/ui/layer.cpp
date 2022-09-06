#include "layer.h"

#include <assert.h>


namespace playos {

Layer::Layer(int x, int y, int width, int height):
    Element(nullptr)
{
    setGeometry(x, y, width, height);
}

void Layer::onEvent(Event &event)
{
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
        (*it)->onEvent(event);
    }
}

void Layer::onUpdate(UIContext *context, float dt)
{
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
        (*it)->update(context, dt);
    }
}

void Layer::onDraw(UIContext *context)
{
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
        (*it)->draw(context);
    }
}

void Layer::add(Element *el) {
    assert(el);
    el->setParent(this);
    m_elements.push_back(el);
}

void Layer::remove(Element *el) {
    for (auto it = m_elements.begin(); it != m_elements.end(); ++it) {
        if (*it == el) {
            el->setParent(nullptr);
            m_elements.erase(it);
            break;
        }
    }
}

void Layer::invalid()
{
    Element::invalid();

    for (auto it : m_elements) {
        it->invalid();
    }
}


}