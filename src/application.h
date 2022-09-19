#ifndef __PLAYOS_APPLICATION_H__
#define __PLAYOS_APPLICATION_H__

#include <chrono>
#include <memory>
#include <SDL.h>
#include <vector>

#include "eventloop/event_loop.h"
#include "ui/director.h"
#include "video_player.h"

namespace playos {

class Application: public Task, public player::ContextExecutor {
public:
    Application(int argc, char **argv);
    ~Application();
    bool init();

    int run();

    std::string getResource(const std::string &res);

private:
    bool initWindow();
    bool fillEvent(Event &ev, SDL_Event &event);
    void scanMedia(const char * const path);
    void tryAddMediaFile(const char *path);

private:
    void run(int events) override;
    void exec(std::function<void ()> func) override;

private:
    int argc;
    const char * const *argv;
    std::string m_resourceDir;
    std::vector<std::string> m_videoUrls;

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
