#include <iostream>

#include "application.h"

#include "spdlog/spdlog.h"


void logger_init()
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::flush_on(spdlog::level::debug);
}


int main(int argc, char *argv[])
{
    logger_init();

    playos::Application app(argc, argv);
    app.init();

    return app.run();
}
