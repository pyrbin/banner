#include <banner/core/engine.hpp>
#include <banner/core/platform.hpp>
#include <banner/gfx/graphics.hpp>
#include <banner/util/debug.hpp>

namespace ban {

void
engine::bootstrap()
{
    debug::log("Initializing engine");

    platform_ = new platform(this, "test");
    graphics_ = new graphics(platform_);

    platform_->start_loop();
}

void
engine::tick(f32 dt) const
{ }

} // namespace ban
