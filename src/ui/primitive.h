#ifndef __PLAYOS_PRIMITIVE_H__
#define __PLAYOS_PRIMITIVE_H__

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "element.h"
#include "color.h"
#include "gl/buffers.h"
#include "gl/shader.h"
#include "gl/shader_manager.h"
#include "gl/texture.h"
#include "ui_context.h"


namespace playos {

class Primitive: public Element {
public:
    enum Type {
        Circle,
        Rectangle,
    };

public:
    Primitive(Type type);
    Primitive(Primitive&&) = default;
    ~Primitive();

    void setGeometry(float x, float y, float width, float height);
    void setColor(int r, int g, int b, int a);
    void setColor(float r, float g, float b, float a);
    void setColor(const Color &color);
    void setTexture(std::shared_ptr<Texture> texture);

    void onUpdate(UIContext *context, float dt) override;
    void onDraw(UIContext *context) override;

    void setRounded(int r);

private:
    Type m_type;
    Color m_color;
    float m_rounded;
    float m_x, m_y, m_width, m_height;

    std::shared_ptr<Shader> m_shader;
    const Buffers * const m_buffers;
    std::shared_ptr<Texture> m_texture;
    glm::mat4 m_model;
};

}

#endif
