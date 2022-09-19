#ifndef _PLAYOS_ICON_BUTTON_H__
#define _PLAYOS_ICON_BUTTON_H__

#include "element.h"
#include "primitive.h"
#include "icon.h"
#include "state.h"

#include <vector>


namespace playos {

class IconButton: public Element {
public:
    class ClickListener {
    public:
        virtual void onClick(IconButton *btn) = 0;
    };

public:
    IconButton(const std::string &icon, Element *parent = nullptr);
    IconButton(IconButton&&) = default;
    ~IconButton();

    void addListener(ClickListener *listener) {
        if (listener) {
            m_clickListeners.push_back(listener);
        }
    }

    void setId(int id) {
        m_id = id;
    }

    int getId() {
        return m_id;
    }

    void setColor(Color color);
    void setActiveColor(Color color);
    void setIconColor(Color color);
    void setActiveIconColor(Color color);
    void setIcon(const std::string &icon);

protected:
    bool onMouseEvent(MouseEvent &event) override;
    void onUpdate(UIContext *context, float dt) override;
    void onDraw(UIContext *context) override;

private:
    int m_id;
    Icon m_icon;
    Primitive m_bg;
    Color m_activeColor;
    Color m_defaultColor;
    Color m_activeIconColor;
    Color m_defaultIconColor;
    State<int> m_state;
    std::vector<ClickListener *> m_clickListeners;
};

}

#endif
