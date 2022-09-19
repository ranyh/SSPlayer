#ifndef __PLAYOS_ELEMENT_H__
#define __PLAYOS_ELEMENT_H__

#include "event.h"
#include "ui_context.h"


namespace playos {

class Element {
public:
    Element(Element *parent):
            m_x(0), m_y(0),
            m_width(0), m_height(0),
            m_parent(parent), m_visible(true) { }
    virtual ~Element() { };

    Element(Element&& other):
             m_x(other.m_x), m_y(other.m_y),
            m_width(other.m_width), m_height(other.m_width),
            m_parent(other.m_parent), m_visible(other.m_visible) {
    }

    Element(const Element&) = delete;
    Element &operator=(const Element&) = delete;

    virtual void onEvent(Event &event) { };
    virtual bool onMouseEvent(MouseEvent &event) { return false; }
    virtual bool onKeyEvent(KeyEvent &event) { return false; }
    virtual void onUpdate(UIContext *context, float dt) = 0;
    virtual void onDraw(UIContext *context) = 0;

    void update(UIContext *context, float dt) {
        if (!m_visible) {
            return;
        }
        onUpdate(context, dt);
    }

    void draw(UIContext *context) {
        if (!m_visible) {
            return;
        }

        m_invalid = false;
        onDraw(context);
    }

    void setPosition(float x, float y) {
        m_x = x;
        m_y = y;

        invalid();
    }

    void setSize(float width, float height) {
        m_width = width;
        m_height = height;
        invalid();
    }

    void setGeometry(float x, float y, float width, float height) {
        m_x = x;
        m_y = y;
        m_width = width;
        m_height = height;
        invalid();
    }

    void setParent(Element *parent) {
        m_parent = parent;
        invalid();
    }

    Element *parent() {
        return m_parent;
    }

    bool isValid() {
        return !m_invalid;
    }

    bool isInvalid() {
        return m_invalid;
    }

    virtual void invalid() {
        m_invalid = true;
    }

    float x() {
        return m_x;
    }

    float y() {
        return m_y;
    }

    float width() {
        return m_width;
    }

    float height() {
        return m_height;
    }

    bool checkPosition(float x, float y) {
        return m_visible && m_x <= x && m_x + m_width >= x &&
                    m_y <= y && m_y + m_height >= y;
    }

    void setVisibility(bool visible) {
        m_visible = visible;
    }

private:
    float m_x, m_y;
    float m_width;
    float m_height;
    Element *m_parent;
    bool m_invalid;
    bool m_visible;
};

}

#endif
