
#include <iostream>
#include <banner/banner.hpp>

using namespace ban;

int
main()
{
    platform* pl = new platform("Hello my dog");
    graphics* gr = new graphics(pl);

    pl->on_update.connect([&]() { 
        
    });

    pl->start_loop();
}
