#pragma once

#include "../common/utility.hpp"
#include <vulkan/vulkan.hpp>

#define assert_vk_success(expr)                                                                    \
    {                                                                                              \
        ASSERT(expr == vk::Result::eSuccess);                                                      \
    }
