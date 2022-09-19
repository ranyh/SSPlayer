#ifndef __PLAYOS_SILIDER_H__
#define __PLAYOS_SILIDER_H__

#include "element.h"
#include "primitive.h"
#include "vec.h"


namespace playos {

class Slider: public Element {
public:
    class SliderListener {
    public:
        virtual void onProgressChanged(float progress) = 0;
        virtual void onStartSeek() = 0;
        virtual void onEndSeek() = 0;
    };

public:
    Slider(Element *parent = nullptr);
    ~Slider();

    void setValue(float value);
    void setProgress(float progress);

    void setListener(SliderListener *listener);

protected:
    void onEvent(Event &event) override;
    bool onMouseEvent(MouseEvent &event) override;
    void onUpdate(UIContext *context, float dt) override;
    void onDraw(UIContext *context) override;

private:
    void _setProgress(float x, float y);

private:
    Primitive m_track;
    Primitive m_activeTrack;
    Primitive m_thumb;

    Color m_activeColor;
    Color m_inactiveColor;
    float m_value;
    float m_progress;
    bool m_onThumbGrab;
    SliderListener *m_listener;
};

}

#endif
