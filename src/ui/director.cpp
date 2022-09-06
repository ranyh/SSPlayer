#include "director.h"

namespace playos {

Director::Director()
{
}

Director::~Director()
{
}

std::shared_ptr<Director> Director::instance()
{
    static std::shared_ptr<Director> director(new Director);

    return director;
}

void Director::init(std::unique_ptr<UIContext> context)
{
    m_context = std::move(context);
}

void Director::push(std::shared_ptr<Scene> scene)
{
    if (scene) {
        m_scenes.emplace_back(scene);
        scene->init(m_context.get(), m_context->drawableWidth(), m_context->drawableHeight());
    }
}

std::shared_ptr<Scene> Director::pop()
{
    if (m_scenes.empty()) {
        return nullptr;
    }

    auto scene = m_scenes.back();
    m_scenes.pop_back();

    return scene;
}

std::shared_ptr<Scene> Director::top()
{
    if (m_scenes.empty()) {
        return nullptr;
    }

    return m_scenes.back();
}

void Director::onEvent(Event &event)
{
    if (event.type == Event::WINDOW_RESIZE) {
        m_context->onWindowResize(event.width, event.height);
        top()->setSize(event.width, event.height);
    }

    if (!m_scenes.empty()) {
        m_scenes.back()->onEvent(event);
    }
}

void Director::update(int dt)
{
    if (!m_scenes.empty()) {
        m_scenes.back()->update(m_context.get(), dt);
    }
}

void Director::draw()
{
    if (!m_scenes.empty()) {
        m_scenes.back()->draw(m_context.get());
    }
}

}
