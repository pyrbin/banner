#pragma once

#include <array>

#include <banner/core/types.hpp>
#include <vulkan/vulkan.hpp>

namespace bnr {
struct vertex
{
    using list = vector<vertex>;

    v2 pos;
    v3 color;
};


} // namespace bnr
