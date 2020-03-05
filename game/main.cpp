#include <banner/banner.hpp>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

using namespace bnr;

int main()
{
    engine* engine;

    {
        engine = new bnr::engine({ "My game :)", { 800, 600 }, "icon.png" });

        engine->run();
    }

    delete engine;

    _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF);
}