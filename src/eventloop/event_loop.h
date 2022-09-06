#ifndef __PLAYOS_EVENT_LOOP_H__
#define __PLAYOS_EVENT_LOOP_H__

#include <atomic>
#include <functional>
#include <mutex>
#include <deque>
#include <thread>


namespace playos {

class Task {
public:
    using TaskFunc = std::function<void(Task *task, int events)>;

    enum {
        EventNone   = 0,
        EventIn     = 1 << 1,
        EventOut    = 1 << 2,
        EventHug    = 1 << 3,
        EventEnd    = 1 << 4
    };

protected:
    Task() { };
    Task(TaskFunc run):_run(run)  { }


public:
    static Task *create(TaskFunc run) {
        return new Task(run);
    }

    static bool isEnd(int events) { return events & EventEnd; }

    virtual void run(int events) {
        _run(this, events);
    }

public:
    int events;

private:
    TaskFunc _run;
};

class EventLoop {
protected:
    virtual int dispatch() = 0;
    virtual int addFdWatchInternal(int fd, Task *task, int events) = 0;
    virtual int removeFdWatchInternal(int fd) = 0;

public:
    EventLoop();
    virtual ~EventLoop();

    int postTask(Task *task);
    int removeTask(Task *task);

    int addFdWatch(int fd, Task *task, int events) {
        return addFdWatchInternal(fd, task, events);
    }
    int removeFdWatch(int fd) {
        return removeFdWatchInternal(fd);
    }

    int run(bool thread = false);
    int stop();

    bool isRunOnCurrentThread();

private:
    int _run();
    int runInThread();
    void waitThread();

protected:
    std::atomic_bool m_running;
    std::mutex m_mutex;
    std::deque<Task *> m_tasksQueue;
    std::once_flag m_runFlag;
    std::thread::id m_threadId;
    std::unique_ptr<std::thread> m_thread;
};

}

#endif
