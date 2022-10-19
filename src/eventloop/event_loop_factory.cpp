#include "event_loop_factory.h"
#include "select_event_loop.h"


namespace playos {

EventLoopFactory &EventLoopFactory::instance()
{
    static EventLoopFactory instance;

    return instance;
}

std::shared_ptr<EventLoop> EventLoopFactory::create()
{
// #ifdef __linux__
//     return std::make_shared;
// #else
    return std::make_shared<SelectEventLoop>();
// #endif
}

}
