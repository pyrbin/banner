#pragma once

#include <banner/core/types.hpp>
#include <vulkan/vulkan.hpp>

namespace bnr {
struct buffer
{
    using list = vector<buffer>;

    vk::Buffer vk_buffer_;
};

} // namespace bnr
