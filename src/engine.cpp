#include "debug.h"
#include "platform.h"
#include "vulkan_render.h"
#include "engine.h"

namespace tde {

engine::engine(const std::string& name)  {
    debug::log("Initializing engine: %s", name.c_str());
    _platform = new platform(this, name);
    _renderer = new vulkan_renderer(_platform);
}

engine::~engine()
{

}

void
engine::bootstrap() const
{
    _platform->start_loop();
}

void
engine::tick(const f32 dt) const
{
    
}

}


