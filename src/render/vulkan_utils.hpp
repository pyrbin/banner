#pragma once

#include "../utility.h"
#include <vulkan/vulkan.hpp>

constexpr void
assert_vk_success(const vk::Result result)
{
    ASSERT(result == vk::Result::eSuccess);
}

constexpr void
assert_vk_success(const VkResult result)
{
    ASSERT(result == VK_SUCCESS);
}