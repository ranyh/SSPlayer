#ifndef __PLAYOS_APPLICATION_H__
#define __PLAYOS_APPLICATION_H__

#include <chrono>
#include <memory>
#include <SDL.h>

#include "eventloop/event_loop.h"
#include "ui/director.h"
#include "video_player.h"

namespace playos {

class Application: public Task, public player::ContextExecutor {
public:
    Application();
    ~Application();
    bool init();

    int run();

private:
    bool initWindow();
    void fillEvent(Event &ev, SDL_Event &event);

private:
    void run(int events) override;
    void exec(std::function<void ()> func) override;

private:
    SDL_Window *window;
    SDL_GLContext glContext;
    SDL_Event sdlEvent;
    Event event;
    std::chrono::time_point<std::chrono::system_clock> lastFrame;
    std::chrono::time_point<std::chrono::system_clock> currentFrame;
    std::shared_ptr<EventLoop> eventLoop;

    std::shared_ptr<Director> m_director;
    std::shared_ptr<VideoPlayer> m_videoPlayer;
};

}

#endif
