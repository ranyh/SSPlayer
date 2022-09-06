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
            m_parent(parent) { }
    virtual ~Element() { };

    Element(Element&& other):
             m_x(other.m_x), m_y(other.m_y),
            m_width(other.m_width), m_height(other.m_width),
            m_parent(other.m_parent) {
    }

    Element(const Element&) = delete;
    Element &operator=(const Element&) = delete;

    virtual void onEvent(Event &event) { };
    virtual void onUpdate(UIContext *context, float dt) = 0;
    virtual void onDraw(UIContext *context) = 0;

    void update(UIContext *context, float dt) {
        onUpdate(context, dt);
    }

    void draw(UIContext *context) {
        m_invalid = false;
        onDraw(context);
    }

    void setPosition(int x, int y) {
        m_x = x;
        m_y = y;

        invalid();
    }

    void setSize(int width, int height) {
        m_width = width;
        m_height = height;
        invalid();
    }

    void setGeometry(int x, int y, int width, int height) {
        setPosition(x, y);
        setSize(width, height);
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

    int x() {
        return m_x;
    }

    int y() {
        return m_y;
    }

    int width() {
        return m_width;
    }

    int height() {
        return m_height;
    }

private:
    int m_x, m_y;
    int m_width;
    int m_height;
    Element *m_parent;
    bool m_invalid;
};

}

#endif
