#ifndef __PLAYOS_SELECT_EVENT_LOOP_H__
#define __PLAYOS_SELECT_EVENT_LOOP_H__

#include "event_loop.h"
#include "event_loop_factory.h"

#include <unordered_map>


namespace playos {

class SelectEventLoop: public EventLoop {
protected:
    int dispatch() override;
    int addFdWatchInternal(int fd, Task *task, int events) override;
    int removeFdWatchInternal(int fd) override;

public:
    SelectEventLoop();

private:
    std::unordered_map<int, Task *> m_fds;
    std::mutex m_fdsMux;
};

}

#endif
