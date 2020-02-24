#pragma once

#include "types.h"

#include <vector>
#include <vulkan/vulkan.h>

namespace tde {

struct vulkan_swapchain_support_details
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentation_modes;
};

class platform;

class vulkan_renderer
{
public:
    vulkan_renderer(platform* platform);
    ~vulkan_renderer();

private:

    VkPhysicalDevice select_physical_device() const;

    bool valid_physical_device(VkPhysicalDevice) const;
    void detect_queue_family_indices(VkPhysicalDevice,i32*,i32*) const;
    vulkan_swapchain_support_details query_swapchain_supports(VkPhysicalDevice) const;
    void create_logical_device(std::vector<const char*>&);
    void create_shader(const char*);
    char* read_shader_file(const char*, const char*, u64*);

    platform* _platform;

    VkInstance _instance;

    VkDebugUtilsMessengerEXT _debug_messenger;

    VkPhysicalDevice _physical_device;
    VkDevice _device;

    i32 _graphics_family_queue_idx;
    i32 _presentation_family_queue_idx;

    VkQueue _graphics_queue;
    VkQueue _presentation_queue;

    VkSurfaceKHR _surface;

    u64 _shader_stage_count;
    std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;
};
}