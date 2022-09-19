#ifndef __PLAYOS_STATE_H__
#define __PLAYOS_STATE_H__

#include <deque>
#include <stdio.h>


namespace playos {

template<typename T>
class State {
    State(const State& state) = delete;
    State &operator=(State& state) = delete;

public:
    // duration in millisecond
    State(const T& t, float duration, bool remainDefault = false): 
            m_current(t), m_duration(duration/1000.0), m_remain(remainDefault),
            m_elapse(0.0f) {
        if (remainDefault) {
            m_lockState = t;
        }
    }
    State(State&&) = default;

    State &operator=(const T &t) {
        set(t);
        return *this;
    }

    bool operator==(const T &t) {
        return m_current == t;
    }

    bool operator!=(const T &t) {
        return m_current != t;
    }

    T& operator()() {
        return m_current;
    }

    T& get() {
        return m_current;
    }

    void set(const T& t) {
        if (m_elapse >= m_duration) {
            m_current = t;
            m_elapse = 0.0f;
        } else {
            m_pendings.push_back(t);
        }
    }

    void update(float dt) {
        m_elapse += dt;
        if (m_elapse >= m_duration) {
            if (!m_pendings.empty()) {
                m_current = m_pendings.front();
                m_pendings.pop_front();
                m_elapse = 0.0f;
            } else if (m_remain) {
                m_current = m_lockState;
                m_elapse = 0.0f;
            }
        }
    }


private:
    std::deque<T> m_pendings;
    T m_lockState;
    T m_current;
    bool m_remain;
    float m_duration;
    float m_elapse;
};

}

#endif
