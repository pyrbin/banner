#include <banner/banner.hpp>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

using namespace ban;

int main()
{
    window* window = new ban::window("My window :)", { 800, 600 }, "icon.png");

    while (!window->should_close()) {
        window->update();
    }

    delete window;

    _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF);
}
