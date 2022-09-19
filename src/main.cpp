#include <iostream>

#include "application.h"


int main(int argc, char *argv[])
{
    playos::Application app(argc, argv);
    app.init();

    return app.run();
}
