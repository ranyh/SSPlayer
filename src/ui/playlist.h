#ifndef __PLAYOS_PLAYLIST_H__
#define __PLAYOS_PLAYLIST_H__

#include <vector>

#include "element.h"
#include "text.h"
#include "primitive.h"
#include "icon_button.h"


namespace playos {

class Playlist: public Element, public IconButton::ClickListener {
public:
    Playlist(Element *parent = nullptr);

    void setList(const std::vector<std::string> &list);
    std::string getItem(int index);

private:
    bool onMouseEvent(MouseEvent &event) override;
    void onUpdate(UIContext *context, float dt) override;
    void onDraw(UIContext *context) override;

    void onClick(IconButton *btn) override;

private:
    Primitive m_bg;
    IconButton m_closeBtn;
    std::vector<std::string> m_list;
    std::vector<Text> m_listView;
    bool m_listUpdated;
};

}

#endif
