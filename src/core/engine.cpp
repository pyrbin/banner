

#include "engine.hpp"

#include "../debug.hpp"
#include "../render/vulkan_renderer.hpp"

#include "platform.hpp"

namespace tde {

engine::engine(const std::string& name)
{
    debug::log("Initializing engine: %s", name.c_str());
    _platform = new platform(this, name);
    _renderer = new vulkan_renderer(_platform);
}

engine::~engine() {}

void
engine::bootstrap() const
{
    _platform->start_loop();
}

void
engine::tick(const f32 dt) const
{
    //_renderer->draw_frame();
}

} // namespace tde
