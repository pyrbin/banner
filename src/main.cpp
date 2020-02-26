// ┌──────────────────────────────────────────────────────────────────┐
// │ TDEngine                                                         │
// └──────────────────────────────────────────────────────────────────┘

#include "common/utility.hpp"
#include "core/engine.hpp"

int main()
{
    const auto engine = new tde::engine(TDE_ENGINE_NAME);

    engine->bootstrap();

    delete engine;
    return 0;
}
