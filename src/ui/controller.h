#ifndef __PLAYOS_CONTROLLER_H__
#define __PLAYOS_CONTROLLER_H__

#include "layer.h"
#include "gl/shader.h"
#include "gl/buffers.h"
#include "gl/texture.h"
#include "text.h"
#include "icon_button.h"
#include "slider.h"
#include "group.h"
#include "temp_state.h"


namespace playos {

class Controller: public Layer, public IconButton::ClickListener,
        public Slider::SliderListener {
public:
    class Listener {
    public:
        virtual void onPlay() = 0;
        virtual void onPause() = 0;
        virtual void onStop() = 0;
        virtual void onSeek(int64_t i) = 0;
        virtual void onSetUri(const std::string &uri) = 0;
        virtual void onShowPlaylist() = 0;
        // TODO: 
        virtual bool isPlaying() = 0;
    };

public:
    Controller(Element *parent);

    void onEvent(Event &event) override;
    bool onMouseEvent(MouseEvent &event) override;
    void onUpdate(UIContext *context, float dt) override;
    void onDraw(UIContext *context) override;

    void setPlayList(std::vector<std::string> *playList);

    void setListener(Listener *l) {
        m_listener = l;
    }

    int getCurrentIndex();
    void setCurrentIndex(int index);
    void next();
    void previous();
    void setDuration(uint64_t duration);
    void setProgress(uint64_t p);
    void setVideoTitle(const std::string &title);

protected:
    void onClick(IconButton *btn) override;

    void onProgressChanged(float progress) override;
    void onStartSeek() override;
    void onEndSeek() override;

private:
    Group m_barGroup;
    Group m_headerGroup;
    Primitive m_bar;
    Primitive m_headerBg;
    Listener *m_listener;
    std::vector<std::string> *m_playList;
    int m_index;
    uint64_t m_duration;
    uint64_t m_progress;
    bool m_isPlaying;
    bool m_playingStateStore;
    TempState<bool> m_showState;

    Text m_time;
    std::vector<IconButton> m_buttons;
    Slider m_slider;
    Icon m_playIcon;
    Text m_title;

    glm::mat4 m_model;
};

}

#endif
