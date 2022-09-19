#include "application.h"

namespace playos {

#define KEY_CASE(key, toKey, _state, ev) \
    case (key): \
        (ev).type = Event::KEY; \
        (ev).value = (toKey); \
        (ev).state = (_state); \
        break

bool Application::fillEvent(Event &ev, SDL_Event &event)
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
    } else if (event.type == SDL_MOUSEMOTION) {
        ev.type = Event::MOUSE_MOVE;
        ev.x = event.motion.x;
        ev.y = event.motion.y;

    } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
        switch (event.button.button) {
        case SDL_BUTTON_LEFT:
            ev.value = MOUSE_BUTTON_LEFT;
            break;
        case SDL_BUTTON_MIDDLE:
            ev.value = MOUSE_BUTTON_MIDDLE;
            break;
        case SDL_BUTTON_RIGHT:
            ev.value = MOUSE_BUTTON_RIGHT;
            break;
        default:
            return false;
        }

        ev.type = Event::MOUSE_BUTTON;
        ev.x = event.button.x;
        ev.y = event.button.y;
        ev.state = event.button.state == SDL_PRESSED ? KEY_PRESSED : KEY_RELEASE;
    } else if (event.type == SDL_MOUSEWHEEL) {
        ev.type = Event::MOUSE_WHEEL;
        ev.x = event.wheel.x;
        ev.y = event.wheel.y;
    } else {
        return false;
    }

    return true;
}

}
