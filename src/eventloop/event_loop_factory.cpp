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
#ifdef __linux__
    return nullptr;
#else
    return std::make_shared<SelectEventLoop>();
#endif
}

}
