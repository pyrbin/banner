#pragma once

#include "types.h"

#include <string>

namespace tde {

class platform;
class vulkan_renderer;

class engine
{
public:
    explicit engine(const std::string&);
    ~engine();

    void bootstrap() const;
    void tick(const f32) const;

private:
    platform* _platform;
    vulkan_renderer* _renderer;
};

} // namespace tde