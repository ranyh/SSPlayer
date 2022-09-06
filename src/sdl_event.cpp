#include "application.h"

namespace playos {

#define KEY_CASE(key, toKey, _state, ev) \
    case (key): \
        (ev).type = Event::KEY; \
        (ev).value = (toKey); \
        (ev).state = (_state); \
        break

void Application::fillEvent(Event &ev, SDL_Event &event)
{
    memset(&ev, 1, sizeof(ev));

    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        switch (event.key.keysym.sym) {
        KEY_CASE(SDLK_UP, KEY_UP, event.type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASE, ev);
        KEY_CASE(SDLK_DOWN, KEY_DOWN, event.type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASE, ev);
        KEY_CASE(SDLK_SPACE, KEY_SPACE, event.type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASE, ev);
        }
    } else if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
            int w, h;
            SDL_GL_GetDrawableSize(window, &w, &h);
            // SDL_GetWindowSize(window, &w, &h);
            ev.type = Event::WINDOW_RESIZE;
            ev.width = w;
            ev.height = h;
        }
    }
}

}
