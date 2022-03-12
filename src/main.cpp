#include "SDL.h"
#include "SDL_events.h"
#include "SDL_video.h"
#include <cstdio>
#include <iostream>

#include "glad/glad.h"

constexpr int WIN_WIDTH = 640;
constexpr int WIN_HEIGHT = 480;


int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_GLContext glContext;
    SDL_Event event;
    bool running = true;

    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) < 0) {
        printf("Failed to init SDL2\n");

        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    window = SDL_CreateWindow("Player", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        printf("Failed to create window: %s\n", strerror(errno));

        goto QUIT_SDL;
    }

    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        printf("Faild to create GL Context: %s\n", strerror(errno));
        goto DESTROY_WINDOW;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        goto DESTROY_GL_CONTEXT;
    }

    glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);

    for (; running; ) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);



        SDL_GL_SwapWindow(window);
    }

DESTROY_GL_CONTEXT:
    SDL_GL_DeleteContext(glContext);
DESTROY_WINDOW:
    SDL_DestroyWindow(window);
QUIT_SDL:
    SDL_Quit();

    return 0;
}
