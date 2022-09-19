#ifndef __PLAYOS_GROUP_H__
#define __PLAYOS_GROUP_H__

#include "element.h"

#include <vector>


namespace playos {

class Group: public Element {
public:
    Group(Element *parent = nullptr);

    void add(Element *el);
    void remove(Element *el);

public:
    virtual void onEvent(Event &event) override;
    virtual bool onMouseEvent(MouseEvent &event) override;
    virtual void onUpdate(UIContext *context, float dt) override;
    virtual void onDraw(UIContext *context) override;

    void invalid() override;

private:
    std::vector<Element *> m_elements;
    Element *m_activeElement;
};

}

#endif
