#include "slider.h"

#include "gl/shader_manager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace playos {

Slider::Slider(Element *parent):
        Element(parent), m_track(Primitive::Rectangle),
        m_activeTrack(Primitive::Rectangle), m_thumb(Primitive::Circle),
        m_activeColor(0.0f, 0.81, 0.99f, 1.0f),
        m_inactiveColor(0.4, 0.4, 0.4f, 1.0f), m_value(1.0f), m_progress(0.0),
        m_onThumbGrab(false), m_listener(nullptr)
{
}

Slider::~Slider()
{

}

void Slider::onEvent(Event &event)
{

}

bool Slider::onMouseEvent(MouseEvent &event)
{
    if (event.type == Event::MOUSE_MOVE) {
        if (m_onThumbGrab) {
            _setProgress(event.x, event.y);
        }
    } else if (event.type == Event::MOUSE_ENTER) {
    } else if (event.type == Event::MOUSE_LEAVE) {
        if (m_onThumbGrab) {
            m_listener->onEndSeek();
            m_onThumbGrab = false;
        }
    } else if (event.type == Event::MOUSE_BUTTON) {
        if (event.value == MOUSE_BUTTON_LEFT) {
            if (!m_thumb.checkPosition(event.x, event.y) && event.state == KEY_PRESSED) {
                _setProgress(event.x, event.y);
            } else {
                m_onThumbGrab = event.state == KEY_PRESSED;
                if (m_listener) {
                    if (m_onThumbGrab) {
                        m_listener->onStartSeek();
                    } else {
                        m_listener->onEndSeek();
                    }
                }
            }
        }
    }

    return true;
}

void Slider::onUpdate(UIContext *context, float dt)
{
    if (!isInvalid()) {
        float x = this->x(), y = this->y();
        float width = this->width();
        float height = this->height();
        float trackHeight = height * 0.5;
        float trackY = y + (height - trackHeight) / 2.0;

        m_track.setRounded(trackHeight/2);
        m_track.setGeometry(x, trackY, width, trackHeight);

        m_activeTrack.setColor(m_activeColor);
        m_activeTrack.setRounded(trackHeight/2);
        m_activeTrack.setGeometry(x, trackY, width * m_progress/m_value, trackHeight);

        m_thumb.setColor(m_activeColor);
        m_thumb.setGeometry(x + (width * m_progress/m_value)-height/2, y, height, height);
    }
}

void Slider::onDraw(UIContext *context)
{
    float progress = m_progress/m_value;

    m_track.draw(context);
    m_activeTrack.draw(context);
    m_thumb.draw(context);
}

void Slider::setValue(float value)
{
    m_value = value;
}

void Slider::setProgress(float progress)
{
    if (!m_onThumbGrab) {
        m_progress = progress;
    }
}

void Slider::setListener(SliderListener *listener)
{
    m_listener = listener;
}

void Slider::_setProgress(float x, float y)
{
    m_progress = (x - this->x())/width() * m_value;
    if (m_progress > m_value) {
        m_progress = m_value;
    } else if (m_progress < 0.0f) {
        m_progress = 0.0f;
    }

    if (m_listener) {
        m_listener->onProgressChanged(m_progress);
    }
}

}
