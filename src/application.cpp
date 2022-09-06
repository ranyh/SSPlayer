#include "application.h"

#include "glad/glad.h"

#include <SDL_events.h>
#include <SDL_video.h>

#include <stdio.h>
#include <iostream>

#include "eventloop/event_loop_factory.h"
#include "ui/font/font_manager.h"
#include "ui/gl/shader.h"


constexpr int DEFAULT_WIN_WIDTH = 640;
constexpr int DEFAULT_WIN_HEIGHT = 480;

namespace playos {

Application::Application():
        lastFrame(std::chrono::system_clock::now()),
        currentFrame(std::chrono::system_clock::now()),
        m_director(Director::instance()),
        m_videoPlayer(nullptr)
{
    eventLoop = EventLoopFactory::instance().create();

    FontManager::instance().addFont("default", "../font/FiraCode-Regular.ttf");
    FontManager::instance().addFont("mdi", "../font/materialdesignicons-webfont.ttf");
}

Application::~Application()
{
    m_videoPlayer = nullptr;
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool Application::init()
{
    if (!initWindow()) {
        return false;
    }

    auto uiContext = std::unique_ptr<UIContext>(
            new UIContext(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT));
    m_director->init(std::move(uiContext));

    m_videoPlayer = std::make_shared<VideoPlayer>(this);
    m_director->push(m_videoPlayer);

    return true;
}

bool Application::initWindow()
{
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        printf("Failed to init SDL2\n");

        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    window = SDL_CreateWindow("Player", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        printf("Failed to create window: %s\n", strerror(errno));

        return false;
    }

    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        printf("Faild to create GL Context: %s\n", strerror(errno));
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    return true;
}

int Application::run()
{
    eventLoop->postTask(this);

    return eventLoop->run();
}

void Application::run(int events)
{
    if (Task::isEnd(events)) {
        return;
    }

    while (SDL_PollEvent(&sdlEvent)) {
        if (sdlEvent.type == SDL_QUIT) {
            eventLoop->stop();
            return;
        } else if (sdlEvent.type == SDL_WINDOWEVENT) {
            if (sdlEvent.window.event == SDL_WINDOWEVENT_RESIZED) {
                int w, h;
                SDL_GL_GetDrawableSize(window, &w, &h);
                // SDL_GetWindowSize(window, &w, &h);
            
                glViewport(0, 0, w, h);
            }
        }

        fillEvent(event, sdlEvent);
        m_director->onEvent(event);
    }

    currentFrame = std::chrono::system_clock::now();
    std::chrono::duration<float> dt = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );

    m_director->update(dt.count());
    m_director->draw();

    SDL_GL_SwapWindow(window);

    eventLoop->postTask(this);
}

void Application::exec(std::function<void ()> func)
{
    eventLoop->postTask(Task::create([func](Task *task, int events) {
        func();

        delete task;
    }));
}

}
