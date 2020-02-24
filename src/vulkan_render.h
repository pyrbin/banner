#pragma once

#include "types.h"

#include <vector>
#include <vulkan/vulkan.h>

namespace tde {
class platform;

class vulkan_renderer
{
public:
    vulkan_renderer(platform* platform);
    ~vulkan_renderer();
private:
    platform* _platform;

    VkInstance _instance;

    VkDebugUtilsMessengerEXT _debug_messenger;

    VkPhysicalDevice _physical_device;
    VkDevice _device;
};
}