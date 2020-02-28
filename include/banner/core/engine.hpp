#pragma once

#include <banner/core/types.hpp>
#include <string>

namespace ban {

class platform;
class graphics;

class engine
{
public:
    explicit engine(){};

    void bootstrap();
    void tick(f32) const;

    graphics* gfx() { return graphics_; };

private:
    platform* platform_;
    graphics* graphics_;
};
} // namespace ban
