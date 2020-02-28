
#include <iostream>
#include <banner/banner.hpp>

using namespace ban;

int
main()
{
    ban::engine* engine = new ban::engine();
    engine->bootstrap();
}
