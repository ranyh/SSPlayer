#ifndef __PLAYOS_LAYER_H__
#define __PLAYOS_LAYER_H__

#include "element.h"

#include <vector>


namespace playos {

class Layer: public Element {
public:
    Layer(int x, int y, int width, int height);

    virtual void onEvent(Event &event) override;
    virtual void onUpdate(UIContext *context, float dt) override;
    virtual void onDraw(UIContext *context) override;

    void add(Element *el);
    void remove(Element *el);

protected:
    void invalid() override;

private:
    std::vector<Element *> m_elements;
};

}

#endif
