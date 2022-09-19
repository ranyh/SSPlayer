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
    VideoView(Element *parent);

    void setTexture(std::shared_ptr<Texture> texture);

protected:
    void onUpdate(UIContext *context, float dt) override;
    void onDraw(UIContext *context) override;

private:
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<Texture> m_frameTexture;
    const Buffers * const m_buffers;
    glm::mat4 m_model;
};

}

#endif
