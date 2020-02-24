#include "debug.h"
#include "engine.h"
#include "utility.h"

int
main()
{
    const auto engine = new tde::engine(CFG_ENGINE_NAME);

    engine->bootstrap();

    delete engine;
    return 0;
}
