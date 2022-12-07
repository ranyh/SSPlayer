#ifndef __PLAYOS_APPLICATION_H__
#define __PLAYOS_APPLICATION_H__

#include <chrono>
#include <memory>
#include <SDL.h>
#include <vector>

#include "eventloop/event_loop.h"
#include "ui/director.h"
#include "video_player.h"
#include "ui/ui_context.h"

#include "spdlog/spdlog.h"


namespace playos {

class SDLResourceContext: public ResourceContext {
public:
    SDLResourceContext(SDL_Window *window, SDL_GLContext context, EGLDisplay eglDisplay):
            m_window(window), m_context(context), m_preContext(nullptr),
            m_eglDisplay(eglDisplay) {
    }

    ~SDLResourceContext() {
        SDL_GL_DeleteContext(m_context);
        SDL_DestroyWindow(m_window);
    }

    int makeCurrent() override {
        m_preContext = SDL_GL_GetCurrentContext();
        int ret = SDL_GL_MakeCurrent(m_window, m_context);
        if (ret != 0) {
            spdlog::error("SDLResourceContext makeCurrent error: {}", SDL_GetError());
        }

        return ret;
    }

    int clearCurrent() override {
        return SDL_GL_MakeCurrent(m_window, m_preContext == m_context ? NULL : m_preContext);
    }

    EGLDisplay getEGLDisplay() override {
        return m_eglDisplay;
    }

private:
    SDL_Window *m_window;
    SDL_GLContext m_context;
    SDL_GLContext m_preContext;
    EGLDisplay m_eglDisplay;
};

class Application: public Task,
        public ResourceContextCreator {
public:
    Application(int argc, char **argv);
    ~Application();
    bool init();

    int run();

    std::string getResource(const std::string &res);
    void exec(std::function<void ()> func);

public:
    ResourceContext *createResourceContext() override;

private:
    bool initWindow();
    bool fillEvent(Event &ev, SDL_Event &event);
    void scanMedia(const char * const path);
    void tryAddMediaFile(const char *path);

private:
    void run(int events) override;

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
