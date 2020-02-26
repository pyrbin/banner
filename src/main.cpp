// ┌──────────────────────────────────────────────────────────────────┐
// │ TDEngine                                                         │
// └──────────────────────────────────────────────────────────────────┘

#include "core/engine.hpp"
#include "debug.hpp"

using namespace tde;

int
main()
{
    tde::engine* engine {nullptr};

    try {
        engine = new tde::engine("Engine");
        engine->bootstrap();
    } catch (const std::exception& exc) {
        debug::fatal(exc.what());
    }

    if (engine != nullptr)
        delete engine;

    return 0;
}
