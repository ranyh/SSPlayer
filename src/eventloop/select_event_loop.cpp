#include "select_event_loop.h"

#include <errno.h>
#include <sys/select.h>


namespace playos {

SelectEventLoop::SelectEventLoop()
{
}

int SelectEventLoop::dispatch()
{
    fd_set readfds, writefds;
    int maxfd = 0;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 5;

    {
        std::lock_guard<std::mutex> lock(m_fdsMux);
        if (m_fds.empty())
            return 0;

        for (auto &entry : m_fds) {
            if (entry.second->events & Task::EventIn) {
                FD_SET(entry.first, &readfds);
                maxfd = std::max(maxfd, entry.first);
            } else if (entry.second->events & Task::EventOut) {
                FD_SET(entry.first, &writefds);
                maxfd = std::max(maxfd, entry.first);
            }
        }
    }

    if (select(maxfd+1, &readfds, &writefds, NULL, &timeout) < 0) {
        if (errno != EAGAIN) {
            return -1;
        }

        return 0;
    }

    std::lock_guard<std::mutex> lock(m_fdsMux);
    for (auto &entry : m_fds) {
        if (FD_ISSET(entry.first, &readfds)) {
            entry.second->run(Task::EventIn);
        } else if (FD_ISSET(entry.first, &writefds)) {
            entry.second->run(Task::EventOut);
        }
    }

    return 0;
}

int SelectEventLoop::addFdWatchInternal(int fd, Task *task, int events)
{
    m_fdsMux.lock();
    m_fds[fd] = task;
    task->events = events;
    m_fdsMux.unlock();

    return 0;
}

int SelectEventLoop::removeFdWatchInternal(int fd)
{
    m_fdsMux.lock();
    Task *task = m_fds[fd];
    m_fds.erase(fd);
    m_fdsMux.unlock();

    task->run(Task::EventEnd);

    return 0;
}

}

