#include <iostream>
#include <time.h>

#include <banner/banner.hpp>

using namespace ban;

int main()
{
    window* window = new ban::window("My window :)", { 800, 600 }, "icon.png");

    while (!window->should_close()) {
        window->update();
    }
}