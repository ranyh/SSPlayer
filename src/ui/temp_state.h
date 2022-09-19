#ifndef __PLAYOS_TEMP_STATE_H__
#define __PLAYOS_TEMP_STATE_H__

namespace playos {

template<typename T>
class TempState {
private:
    TempState(const TempState& state) = delete;
    TempState &operator=(TempState& state) = delete;

public:
    /* newStateRemaintime in millisecond */
    TempState(const T &t, int newStateRemaintime):
            m_default(t), m_current(t), m_remainTime(newStateRemaintime/1000.0),
            m_elapse(0.0f) { }

    TempState(TempState&&) = default;

    TempState &operator=(const T &t) {
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
        m_current = t;
        m_elapse = 0.0f;
    }

    void update(float dt) {
        m_elapse += dt;
        if (m_elapse >= m_remainTime) {
            m_current = m_default;
            m_elapse = 0.0f;
        }
    }

private:
    T m_default;
    T m_current;
    float m_remainTime;
    float m_elapse;
};

}

#endif
