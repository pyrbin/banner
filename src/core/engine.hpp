#pragma once

#include "../common/types.hpp"
#include <string>

namespace tde {

class platform;
class vulkan_graphics;
class renderer;
class tri_renderer;

class engine
{
public:
    explicit engine(const std::string&);
    ~engine();

    void bootstrap() const;
    void tick(f32) const;

    vulkan_graphics* graphics() { return _graphics; };

private:
    platform* _platform;

    vulkan_graphics* _graphics;
    renderer* ren;
    tri_renderer* tri;
};
} // namespace tde
