#include "engine.hpp"

#include "../debug.hpp"
#include "../render/vulkan_graphics.hpp"
#include "../tri_renderer.hpp"
#include "../renderer.hpp"

#include "platform.hpp"

namespace tde {
engine::engine(const std::string& name)
{
    debug::log("Initializing engine: %s", name.c_str());
    _platform = new platform(this, name);
    _graphics = new vulkan_graphics(_platform);
    ren = new renderer(_graphics);
    tri = new tri_renderer(_graphics, ren);
}

engine::~engine() {}

void
engine::bootstrap() const
{
    _platform->start_loop();
}

void
engine::tick(f32 dt) const
{
    ren->pre_update(dt);
    ren->update(dt);
    tri->update(dt);
}
} // namespace tde