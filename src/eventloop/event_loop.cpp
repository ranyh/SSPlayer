#include "event_loop.h"

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>


static volatile int sig;

static void signal_handler(int s)
{
    sig = s;
}

namespace playos {

EventLoop::EventLoop(): m_running(false),
        m_thread(nullptr), m_mainTask(nullptr)
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}

EventLoop::~EventLoop()
{
    stop();
}

int EventLoop::postTask(Task *task)
{
    m_mutex.lock();
    m_tasksQueue.push_back(task);
    m_mutex.unlock();

    return 0;
}

int EventLoop::removeTask(Task *task)
{
    m_mutex.lock();
    for (auto it = m_tasksQueue.begin(); it != m_tasksQueue.end(); ) {
        if (*it == task) {
            m_tasksQueue.erase(it);
        }
    }
    m_mutex.unlock();

    return 0;
}

int EventLoop::run(bool thread)
{
    if (thread) {
        std::call_once(m_runFlag, &EventLoop::runInThread, this);
    } else {
        std::call_once(m_runFlag, &EventLoop::_run, this);
    }

    return 0;
}

int EventLoop::_run()
{
    Task *task;
    std::deque<Task *> tasksQueue;
    int ret = 0;

    m_running = true;
    m_threadId = std::this_thread::get_id();

    while (m_running.load()) {
        if (sig == SIGINT || sig == SIGTERM) {
            break;
        }

        if (m_mainTask) {
            m_mainTask->run(Task::EventNone);
        }

        {
            m_mutex.lock();
            std::swap(tasksQueue, m_tasksQueue);
            m_mutex.unlock();

            while (!tasksQueue.empty()) {
                task = tasksQueue.front();
                task->run(Task::EventNone);
                tasksQueue.pop_front();
            }
        }

        if (dispatch() != 0) {
            ret = -1;
            break;
        }
    }
    m_running = false;

    return ret;
}

int EventLoop::runInThread()
{
    m_thread.reset(new std::thread(&EventLoop::_run, this));
    return 0;
}

void EventLoop::waitThread()
{
    if (m_thread && !isRunOnCurrentThread() && m_thread->joinable()) {
        m_thread->join();
    }
}

int EventLoop::stop()
{
    if (m_running.exchange(false)) {
        dispatch();
    }

    waitThread();

    return 0;
}

void EventLoop::setMainTask(Task *task)
{
    m_mainTask = task;
}

bool EventLoop::isRunOnCurrentThread()
{
    return std::this_thread::get_id() == m_threadId;
}

}
