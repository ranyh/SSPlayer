#ifndef __PLAYOS_FRAME_POOL_H__
#define __PLAYOS_FRAME_POOL_H__

#include <vector>
#include <memory>
#include <functional>


namespace playos {
namespace player {

template<typename T>
T *defaultCreator()
{
    return new T;
}

struct FrameLink {
    FrameLink *prev;
    FrameLink *next;

    int val;
};

template<typename T>
class FramePool {
public:
    using frameCreator = std::function<T *()>;
    static std::unique_ptr<FramePool> create(int size, frameCreator creator = defaultCreator<T>) {
        return std::unique_ptr<FramePool>(new FramePool(size, creator));
    }

    ~FramePool() {
        FrameLink *link, *t;

        for (link = m_frames.next, t = link->next; link != &m_frames; link = t, t = t->next) {
            delete (T *)link;
        }
        m_frames.prev = m_frames.next = &m_frames;


        for (link = m_usingFrames.next, t = link->next; link != &m_usingFrames; link = t, t = t->next) {
            delete (T *)link;
        }
        m_usingFrames.prev = m_usingFrames.next = &m_usingFrames;
    }

    /**
     * @brief Get an T object from pool. If no free object create new one.
     * 
     * @return T* 
     */
    T *getFrame() {
        std::lock_guard<std::mutex> locker(m_mux);
        return _getFrame();
    }

    /**
     * @brief Get one free object from pool, or waiting for free object available.
     * 
     * @return T* 
     */
    T *waitForFrame() {
        std::unique_lock<std::mutex> locker(m_mux);
        if (m_usingSize < m_size) {
            return _getFrame();
        }

        m_freeAvailable.wait(locker, [&]() { return m_usingSize < m_size; });
        return _getFrame();
    }

    void putFrame(T *frame) {
        m_mux.lock();

        // Remove from using frames link
        remove(frame);
        --m_usingSize;

        // Add to frames link
        insert(&m_frames, frame);

        m_mux.unlock();
        m_freeAvailable.notify_one();
    }

    bool isEmpty() {
        std::lock_guard<std::mutex> locker(m_mux);
        return m_usingSize >= m_size;
    }

    size_t size() {
        return m_usingSize;
    }

    size_t capacity() {
        return m_size;
    }

private:
    FramePool(int size, frameCreator creator):
            m_size(size), m_usingSize(0), m_frameCreator(creator) {
        m_usingFrames.prev = m_usingFrames.next = &m_usingFrames;
        m_frames.prev = m_frames.next = &m_frames;

        for (int i = 0; i < size; i++) {
            FrameLink *frame = m_frameCreator();
            insert(&m_frames, frame);
        }
    }

    FramePool(const FramePool&) = delete;
    FramePool(FramePool&&) = delete;

    void insert(FrameLink *link, FrameLink *frame) {
        frame->next = link->next;
        frame->prev = link;

        link->next->prev = frame;
        link->next = frame;

    }

    void remove(FrameLink *frame) {
        frame->prev->next = frame->next;
        frame->next->prev = frame->prev;
    }

    T *_getFrame() {
        FrameLink *frame;
        if (&m_frames == m_frames.next) {
            frame = m_frameCreator();
            ++m_size;
        } else {
            frame = m_frames.next;
            // Remove from frames link
            remove(frame);
        }

        // Add to using frames link
        insert(&m_usingFrames, frame);
        ++m_usingSize;

        return (T *)frame;
    }

private:
    FrameLink m_frames;
    FrameLink m_usingFrames;
    int m_size, m_usingSize;
    frameCreator m_frameCreator;
    std::mutex m_mux;
    std::condition_variable m_freeAvailable;
};

}
}

#endif
