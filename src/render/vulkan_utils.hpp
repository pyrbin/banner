#pragma once

#include "../utility.h"
#include <vulkan/vulkan.hpp>

#define assert_vk_success(expr)                                                                    \
    {                                                                                              \
        ASSERT(expr == vk::Result::eSuccess);                                                      \
    }
