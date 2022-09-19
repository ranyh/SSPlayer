#include "playlist.h"

namespace playos {

Playlist::Playlist(Element *parent):
        Element(parent), m_listUpdated(false),
        m_bg(Primitive::Rectangle), m_closeBtn("mdi-close", this)
{
    m_closeBtn.addListener(this);
}

void Playlist::setList(const std::vector<std::string> &list)
{
    m_list = std::move(list);
    m_listUpdated = true;
}

std::string Playlist::getItem(int index)
{
    if (index < 0 && index > m_list.size()) {
        return "";
    }

    return m_list[index];
}

bool Playlist::onMouseEvent(MouseEvent &event)
{
    if (m_closeBtn.checkPosition(event.x, event.y))
        return ((Element *)&m_closeBtn)->onMouseEvent(event);

    return false;
}

void Playlist::onUpdate(UIContext *context, float dt)
{
    if (isInvalid()) {
        float width = 240.0;
        setGeometry(context->drawableWidth() - width, 0, width, context->drawableHeight());

        m_bg.setGeometry(context->drawableWidth() - width, 0, width, context->drawableHeight());
        m_bg.setColor(0.0, 0.0, 0.0, 0.9f);
        m_bg.update(context, dt);

        m_closeBtn.setGeometry(x() + width - 26.0f, y() + 8.0f, 18, 18);
        m_closeBtn.update(context, dt);
    }

    if (m_listUpdated) {
        float x = this->x(), y = this->y() + 32;
        float width = this->width(), height = 32.0f;

        m_listView.clear();
        for (auto &s : m_list) {
            m_listView.emplace_back(this, s);
            Text *t = &m_listView.back();

            t->setGeometry(x, y, width, height);
            t->setColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
            t->setFontSize(12);

            y += height;
        }
    }

    for (auto &v : m_listView) {
        v.update(context, dt);
    }
}

void Playlist::onDraw(UIContext *context)
{
    m_bg.draw(context);

    for (auto &v : m_listView) {
        v.draw(context);
    }

    m_closeBtn.draw(context);
}

void Playlist::onClick(IconButton *btn)
{
    setVisibility(false);
}

}
