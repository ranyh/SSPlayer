#include <iostream>

#include "application.h"


int main(int argc, char *argv[])
{
    playos::Application app;
    app.init();

    return app.run();
}
