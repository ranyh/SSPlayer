#include "controller.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <unordered_map>

#include "gl/shader_manager.h"


namespace playos {

Controller::Controller(Element *parent):
        Layer(0, 0, 0, 0, parent), m_bar(Primitive::Rectangle), m_headerBg(Primitive::Rectangle),
        m_time(this), m_slider(this), m_playList(nullptr), m_index(0),
        m_playIcon(this, "mdi-play-circle-outline"), m_isPlaying(false),
        m_duration(0), m_progress(0), m_showState(false, 3*1000), m_title(this, "")
{
    setSize(parent->width(), parent->height());
    m_showState = true;

    m_time.setText("00:00/00:00");
    m_time.setColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    m_time.setFontSize(12);

    m_slider.setListener(this);

    m_barGroup.add(&m_bar);
    m_barGroup.add(&m_time);
    m_barGroup.add(&m_slider);

    m_buttons.emplace_back("mdi-skip-previous", this);
    m_buttons.emplace_back("mdi-play", this);
    m_buttons.emplace_back("mdi-skip-next", this);
    m_buttons.emplace_back("mdi-stop", this);
    m_buttons.emplace_back("mdi-playlist-play", this);
    for (int i = 0; i < m_buttons.size(); i++) {
        m_buttons[i].setIconColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
        m_buttons[i].setId(i);
        m_buttons[i].addListener(this);
        m_barGroup.add(&m_buttons[i]);
    }

    add(&m_barGroup);
    add(&m_playIcon);

    m_headerGroup.add(&m_headerBg);
    m_headerGroup.add(&m_title);
    add(&m_headerGroup);
}

void Controller::onEvent(Event &event)
{
    Layer::onEvent(event);

    if (event.type == Event::WINDOW_RESIZE) {
        setSize(parent()->width(), parent()->height());
    }
}

bool Controller::onMouseEvent(MouseEvent &event)
{
    m_showState = true;

    return Layer::onMouseEvent(event);
}

void Controller::onUpdate(UIContext *context, float dt)
{
    if (isInvalid()) {
        float width = this->width();
        float height = this->height();
        float barHeight = 32;
        float barY = parent()->height() - barHeight;
        int timeWidth = 100;

        m_model        = glm::mat4(1.0f);

        setPosition(0.0f, 0.0f);

        m_barGroup.setGeometry(0.0f, barY, width, barHeight);
        m_bar.setGeometry(0.0f, barY, width, barHeight);

        int size = 24, i;
        int _x = x() + 12, _y = barY + (barHeight - size) / 2;
        for (i = 0; i < m_buttons.size() - 1; ++i) {
            auto &_buttons = m_buttons[i];
            _buttons.setGeometry(_x, _y, size, size);
            _x = _x + 24;
        }

        m_buttons[i].setGeometry(x() + width - size *2, _y, size, size);

        int sliderX = _x + 16;
        int sliderWidth = m_buttons[i].x() - sliderX - timeWidth;
        m_slider.setGeometry(sliderX, barY + (barHeight - 12) / 2, sliderWidth, 12);

        m_time.setGeometry(m_slider.x() + sliderWidth, barY - 2, timeWidth, barHeight);
        m_time.setAlignment(Text::AlignCenter);

        m_playIcon.setPosition(x() + (width - 48)/2, y() + (height - 48)/2);
        m_playIcon.setColor(Color(1.0, 1.0, 1.0, 0.8));
        m_playIcon.setSize(48);

        m_headerBg.setColor(Color(0.0, 0.0, 0.0, 0.2));
        m_headerBg.setGeometry(0, 0, width, 32);
        m_title.setColor(Color(1.0, 1.0, 1.0, 1.0));
        m_title.setGeometry(16, 0, width, 32);
        m_title.setFontSize(14);
        m_headerGroup.setGeometry(0, 0, width, 32);
    }

    char time[24];
    snprintf(time, 24, "%02.0f:%02.0f/%02.0f:%02.0f",
            m_duration/(1000.0f*60.0f), m_duration%(1000*60)/1000.0f,
            m_progress/(1000.0f*60.0f), m_progress%(1000*60)/1000.0f);
    m_time.setText(time);

    m_bar.setColor(Color(1.0f, 1.0f, 1.0f, 0.6f));
    for (auto &i : m_buttons) {
        i.setIconColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    }

    if (m_isPlaying != m_listener->isPlaying()) {
        m_isPlaying = m_listener->isPlaying();
        m_buttons[1].setIcon(m_isPlaying ? "mdi-pause" : "mdi-play");
        m_playIcon.setVisibility(!m_isPlaying);
    }

    Layer::onUpdate(context, dt);

    m_showState.update(dt);
    m_barGroup.setVisibility(m_showState.get());
    m_headerGroup.setVisibility(m_showState.get());
}

void Controller::onDraw(UIContext *context)
{
    Layer::onDraw(context);
}

int Controller::getCurrentIndex()
{
    return m_index;
}

void Controller::setCurrentIndex(int index)
{
    m_index = index;
}

void Controller::next()
{
    if (m_playList && !m_playList->empty() && m_index < m_playList->size() - 1) {
        m_listener->onStop();
        m_listener->onSetUri(m_playList->at(++m_index));
        m_listener->onPlay();
    }
}

void Controller::previous()
{
    if (m_playList && !m_playList->empty() && m_index > 0) {
        m_listener->onStop();
        m_listener->onSetUri(m_playList->at(--m_index));
        m_listener->onPlay();
    }
}

void Controller::setDuration(uint64_t duration)
{
    m_slider.setValue(duration);
    m_duration = duration;
}

void Controller::setProgress(uint64_t p)
{
    m_slider.setProgress(p);
    m_progress = p;
}

void Controller::onClick(IconButton *btn)
{
    if (!m_listener) {
        return;
    }

    switch (btn->getId()) {
    case 0:
        previous();
        break;
    case 2:
        next();
        break;
    case 1:
        if (m_isPlaying) {
            m_listener->onPause();
        } else {
            m_listener->onPlay();
        }
        break;
    case 3:
        m_listener->onStop();
        break;
    case 4:
        m_listener->onShowPlaylist();
        break;
    }
}

void Controller::onProgressChanged(float progress)
{
    m_listener->onSeek(progress * 1000000);
}

void Controller::onStartSeek()
{
    if (m_isPlaying) {
        m_playingStateStore = true;
        m_listener->onPause();
    }
}

void Controller::onEndSeek()
{
    if (m_playingStateStore) {
        m_listener->onPlay();
        m_playingStateStore = false;
    }
}

void Controller::setPlayList(std::vector<std::string> *playList)
{
    m_playList = playList;
    if (m_playList && !m_playList->empty()) {
        m_listener->onSetUri(m_playList->at(0));
        m_listener->onPlay();
    }
}

void Controller::setVideoTitle(const std::string &title)
{
    if (title.empty()) {
        m_title.setText("Playing: " + m_playList->at(m_index));
    } else {
        m_title.setText("Playing: " + title);
    }

}

}
