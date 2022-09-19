#ifndef _PLAYOS_BUTTON_H__
#define _PLAYOS_BUTTON_H__

#include "element.h"
#include "primitive.h"
#include "text.h"
#include "state.h"

#include <vector>


namespace playos {

class Button: public Element {
public:
    class ClickListener {
    public:
        virtual void onClick(Button *btn) = 0;
    };

public:
    Button(const std::string &text, Element *parent = nullptr);
    ~Button();

    void addListener(ClickListener *listener) {
        if (listener) {
            m_clickListeners.push_back(listener);
        }
    }

    void setColor(const Color& color);
    void setActiveColor(const Color& color);
    void setTextColor(const Color& color);
    void setActiveTextColor(const Color& color);

protected:
    bool onMouseEvent(MouseEvent &event) override;
    void onUpdate(UIContext *context, float dt) override;
    void onDraw(UIContext *context) override;

private:
    Text m_text;
    Primitive m_bg;
    Color m_activeColor;
    Color m_defaultColor;
    Color m_activeTextColor;
    Color m_defaultTextColor;
    State<int> m_state;
    std::vector<ClickListener *> m_clickListeners;
};

}

#endif
