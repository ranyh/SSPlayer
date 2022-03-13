#ifndef __PLAYOS_PLAYER_H__
#define __PLAYOS_PLAYER_H__

#include "SDL_events.h"
#include "shader.h"
#include <memory>

namespace playos {

class Player {
private:
    std::shared_ptr<Shader> m_shader;
    std::string m_uri;
    int m_tWidth;
    int m_tHeight;

public:
    Player(const std::shared_ptr<Shader> &shader);
    ~Player();

    void setUri(const std::string &uri);

    void update(SDL_Event &event, float dt);
    void render();
};

}

#endif
