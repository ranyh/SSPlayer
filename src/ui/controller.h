#ifndef __PLAYOS_CONTROLLER_H__
#define __PLAYOS_CONTROLLER_H__

#include "element.h"
#include "gl/shader.h"
#include "gl/buffers.h"
#include "gl/texture.h"
#include "text.h"
#include "icon.h"


namespace playos {

class Controller: public Element {
public:
    Controller(Element *parent);

    void onEvent(Event &event) override;
    void onUpdate(UIContext *context, float dt) override;
    void onDraw(UIContext *context) override;

private:
    std::shared_ptr<Shader> m_shader;
    const Buffers * const buffers;

    Text text;
    std::vector<Icon> icon;

    glm::mat4 m_model;
};

}

#endif
