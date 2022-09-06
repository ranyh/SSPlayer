#include "scene.h"

namespace playos {

Scene::Scene(): Layer(0, 0, 0, 0)
{
}

Scene::~Scene()
{
}

void Scene::init(UIContext *context, int width, int height)
{
    std::call_once(m_initFlag, [&]() {
        setSize(width, height);
        onInit(context);
    });
}

}
