#ifndef __PLAYOS_EVENT_LOOP_FACTORY_H__
#define __PLAYOS_EVENT_LOOP_FACTORY_H__

#include "event_loop.h"

#include <memory>

namespace playos {

class EventLoopFactory {
public:
    static EventLoopFactory &instance();
    ~EventLoopFactory() { }

    std::shared_ptr<EventLoop> create();
};

}

#endif
