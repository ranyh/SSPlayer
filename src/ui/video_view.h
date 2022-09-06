#ifndef __PLAYOS_Video_VIEW_H__
#define __PLAYOS_Video_VIEW_H__

#include <memory>

#include "element.h"
#include "gl/shader.h"
#include "gl/texture.h"
#include "gl/buffers.h"


namespace playos {

class VideoView: public Element {
public:
    enum State {
        STOPED,
        LOADING,
        PLAYING,
        PAUSED
    };

public:
    VideoView(Element *parent);

    void setTexture(std::shared_ptr<Texture> texture);

    void setState(State state);

    void play() {
        setState(PLAYING);
    }

    void pause() {
        setState(PAUSED);
    }

    void stop() {
        setState(STOPED);
    }

protected:
    void onEvent(Event &event) override;
    void onUpdate(UIContext *context, float dt) override;
    void onDraw(UIContext *context) override;

private:
    std::shared_ptr<Shader> m_shader;
    std::unique_ptr<Texture> m_stopedTexture;
    std::shared_ptr<Texture> m_frameTexture;
    const Buffers * const m_buffers;
    glm::mat4 m_model;
    State m_state;
};

}

#endif
