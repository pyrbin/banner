#pragma once

#include <vulkan/vulkan.h>
#include "utility.h"

template<typename Expr>
constexpr void
assure_vk(Expr expr)
{
    ASSERT(expr == VK_SUCCESS);
}