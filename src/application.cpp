#include "application.h"

#include "glad/glad.h"

#include <SDL_events.h>
#include <SDL_video.h>

#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "eventloop/event_loop_factory.h"
#include "ui/gl/shader.h"


constexpr int DEFAULT_WIN_WIDTH = 640;
constexpr int DEFAULT_WIN_HEIGHT = 480;

namespace playos {

static const char *s_supportedExts[] = {
    "mp4",
    "mkv",
    "flv",
};
static const int s_supportedExtCount = 3;

static void printHelp(const char *program)
{
    std::cout << "Usage: " << program << " [options] [media file or dir]" << R"(

options:
    -r | --resourceDir <resourceDir>    set the resource dir
)" << "\n";

    exit(0);
}

Application::Application(int argc, char **argv):
        argc(argc), argv(argv), m_resourceDir("./resources"),
        lastFrame(std::chrono::system_clock::now()),
        currentFrame(std::chrono::system_clock::now()),
        m_director(Director::instance()),
        m_videoPlayer(nullptr)
{
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            scanMedia(argv[i]);
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--resourceDir") == 0) {
            i++;
            m_resourceDir = std::string(argv[i], strlen(argv[i]));
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printHelp(argv[0]);
        }
    }

    eventLoop = EventLoopFactory::instance().create();
    eventLoop->setMainTask(this);
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
            new UIContext(m_resourceDir, DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT));
    m_director->init(std::move(uiContext));

    m_videoPlayer = std::make_shared<VideoPlayer>(this);
    m_videoPlayer->setPlayList(m_videoUrls);

    m_director->push(m_videoPlayer);

    return true;
}

bool Application::initWindow()
{
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        printf("Failed to init SDL2: %s, %s\n", SDL_GetError());

        return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1); 
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
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

#ifdef __APPLE__
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
#else
    if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress)) {
#endif
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    return true;
}

#define MAX_PATH 128
void Application::scanMedia(const char * const path)
{
    struct stat st;
    char _path[MAX_PATH];
    int pathLen = strlen(path);

    if (stat(path, &st) != 0) {
        printf("Failed to load media (%s): %s\n", path, strerror(errno));
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        if (pathLen >= MAX_PATH) {
            return;
        }

        struct dirent *dp;
        DIR *dirp = opendir(path);
        if (dirp == NULL) {
            return;
        }

        _path[0] = '\0';
        strncat(_path, path, MAX_PATH);
        if (_path[pathLen-1] != '/') {
            strcat(_path, "/");
        } else {
            --pathLen;
        }

        while ((dp = readdir(dirp)) != NULL) {
            if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
                continue;
            }

#ifdef _DIRENT_HAVE_D_NAMLEN
            int namlen = dp->d_namlen;
#else
            int namlen = _D_EXACT_NAMLEN(dp);
#endif
            _path[pathLen + 1] = '\0';
            if (pathLen + 1 + namlen >= MAX_PATH) {
                continue;
            }

            strcat(_path, dp->d_name);
            if (dp->d_type & DT_DIR) {
                scanMedia(_path);
            } else if (dp->d_type & DT_REG) {
                tryAddMediaFile(_path);
            }
        }

        closedir(dirp);
    } else {
        tryAddMediaFile(path);
    }
}

void Application::tryAddMediaFile(const char *path)
{
    const char *dot = strrchr(path, '.');
    if (dot == NULL || dot == path) {
        return;
    }

    for (int i = 0; i < s_supportedExtCount; ++i) {
        if (strcmp(dot+1, s_supportedExts[i]) == 0) {
            char _path[256];
            realpath(path, _path);
            m_videoUrls.emplace_back(_path, strlen(_path));
        }
    }
}

int Application::run()
{
    return eventLoop->run();
}

std::string Application::getResource(const std::string &res)
{
    if (res.empty()) {
        return m_resourceDir;
    }

    if (res[0] == '/') {
        return m_resourceDir + res;
    }

    return m_resourceDir + "/" + res;
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
        } else if (sdlEvent.type == SDL_KEYDOWN) {
            if (sdlEvent.key.keysym.sym == SDLK_o) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else if (sdlEvent.key.keysym.sym == SDLK_i) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }

        if (fillEvent(event, sdlEvent)) {
            m_director->onEvent(event);
        }
    }

    currentFrame = std::chrono::system_clock::now();
    std::chrono::duration<float> dt = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // glEnable(GL_CULL_FACE);
    // glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_director->update(dt.count());
    m_director->draw();

    SDL_GL_SwapWindow(window);
}

void Application::exec(std::function<void ()> func)
{
    eventLoop->postTask(Task::create([func](Task *task, int events) {
        func();

        delete task;
    }));
}

}
