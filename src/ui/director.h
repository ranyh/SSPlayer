#ifndef __PLAYOS_DIRECTOR_H__
#define __PLAYOS_DIRECTOR_H__

#include <memory>
#include <vector>

#include "scene.h"


namespace playos {

class Director {
private:
    Director();

    Director(const Director &) = delete;
    Director &operator=(const Director &) = delete;

public:
    static std::shared_ptr<Director> instance();
    ~Director();

    void init(std::unique_ptr<UIContext> context);

    void push(std::shared_ptr<Scene> scene);
    std::shared_ptr<Scene> pop();
    std::shared_ptr<Scene> top();

    void onEvent(Event &event);
    void update(float dt);
    void draw();

private:
    std::unique_ptr<UIContext> m_context;
    std::vector<std::shared_ptr<Scene>> m_scenes;
};

}

#endif
