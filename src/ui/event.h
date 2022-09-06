#ifndef __PLAYOS_EVENT_H__
#define __PLAYOS_EVENT_H__

namespace playos {

enum KeyValue {
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,

    KEY_ESC,
    KEY_SPACE,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
};

enum KeyState {
    KEY_PRESSED,
    KEY_RELEASE,
};

enum MouseButton {
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,
};

class Event {
public:
    enum Type {
        KEY,
        MOUSE_BUTTON,
        MOUSE_WHEEL,
        MOUSE_MOVE,
        WINDOW_RESIZE,
    };

public:
    Type type;

    union {
        struct { // For key or button
            int value;
            int state;
        };

        struct { // For window resize
            int width;
            int height;
        };

        struct { // For mouse move
            int x, y;
        };
    };
};

}

#endif
