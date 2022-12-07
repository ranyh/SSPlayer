#ifndef __PLAYOS_PLAYER_STATE_H__
#define __PLAYOS_PLAYER_STATE_H__

#include <functional>
#include <unordered_map>
#include "spdlog/spdlog.h"

namespace playos {
namespace player {

enum PlayerState {
    NIL,
    CREATED,
    LOADING,
    PLAYING,
    PAUSED,
    STOPPED,
};

enum PlayerAction {
    NEW,
    PLAY,
    PAUSE,
    STOP,
    SEEK,
};

}
}

namespace std {
template <> struct hash<playos::player::PlayerState> {
    size_t operator() (const playos::player::PlayerState& s) const { return size_t(s); }
};

template <> struct hash<playos::player::PlayerAction> {
    size_t operator() (const playos::player::PlayerAction& a) const { return size_t(a); }
};
}

namespace playos {
namespace player {

class Fsm {
public:
    Fsm(): m_state(NIL), m_pendingState(NIL) {
        m_stateMap[NIL] = {
            { NEW, CREATED },
        };
        m_stateMap[CREATED] = {
            { PLAY, LOADING },
        };
        m_stateMap[LOADING] = {
            { PLAY, PLAYING },
        };
        m_stateMap[PLAYING] = {
            { PAUSE, PAUSED },
            { STOP, STOPPED },
            { NEW, CREATED },
            { SEEK, PLAYING },
        };
        m_stateMap[PAUSED] = {
            { PLAY, PLAYING },
            { STOP, STOPPED },
            { NEW, CREATED },
            { SEEK, PAUSED },
        };
        m_stateMap[STOPPED] = {
            { PLAY, PLAYING },
            { NEW, CREATED },
        };
    }
    Fsm(const Fsm&) = delete;
    Fsm(Fsm&&) = delete;

    PlayerState current() {
        return m_state;
    }

    bool next(PlayerAction action) {
        if (m_stateMap[m_state].count(action)) {
            m_pendingState = m_stateMap[m_state][action];
            return true;
        }

        return false;
    }

    bool commit() {
        if (m_pendingState == NIL) {
            return false;
        }

        spdlog::debug("Current state: {}, pre state: {}", m_pendingState, m_state);
        m_state = m_pendingState;
        m_pendingState = NIL;
        return true;
    }

private:
    PlayerState m_state;
    PlayerState m_pendingState;

    std::unordered_map<PlayerState, std::unordered_map<PlayerAction, PlayerState>> m_stateMap;
};

}
}

#endif
