#pragma once

#include "../types.h"

#include <vector>
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <optional>

using std::vector;

namespace tde {

class platform;

struct queue_family_indices
{
    std::optional<u32> graphics_family;
    std::optional<u32> present_family;

    [[nodiscard]] bool is_same() const
    {
        return has_values() && graphics_family.value() == present_family.value();
    }

    [[nodiscard]] bool has_values() const
    {
        return graphics_family.has_value() && present_family.has_value();
    }
};

struct swapchain_support_details
{
    vk::SurfaceCapabilitiesKHR capabilities;
    vector<vk::SurfaceFormatKHR> formats;
    vector<vk::PresentModeKHR> present_modes;
};

class vulkan_renderer
{
public:
    explicit vulkan_renderer(platform* platform);
    ~vulkan_renderer();

private:
    platform* _platform;

    vk::UniqueInstance _instance;
    vk::UniqueSurfaceKHR _surface;
    vk::PhysicalDevice _physical_device;
    vk::UniqueDevice _device;

    vk::Queue _graphics_queue;
    vk::Queue _present_queue;

    vk::SwapchainKHR _swapchain;
    vk::SwapchainKHR _old_swapchain{ nullptr };
    vector<vk::Image> _swapchain_buffers{};
    vector<vk::ImageView> _swapchain_buffer_views{};

    vector<const char*> _device_extensions{};
    vector<const char*> _instance_layers{};
    vector<const char*> _instance_extensions{};

    void get_window_info();
    void init_instance();
    void init_surface();
    void pick_physical_device();
    void create_logical_device();
    void create_swapchain();
    void create_swapchain_images();

    bool is_device_suitable(const vk::PhysicalDevice&) const;
    queue_family_indices detect_queue_families(const vk::PhysicalDevice&) const;
    bool check_extension_support(const vk::PhysicalDevice&) const;
    swapchain_support_details query_swapchain_details(const vk::PhysicalDevice&) const;

    vk::SurfaceFormatKHR select_swap_format(
      const std::vector<vk::SurfaceFormatKHR>&) const;
    vk::PresentModeKHR select_swap_mode(
      const std::vector<vk::PresentModeKHR>&) const;
    vk::Extent2D select_swap_extent(const vk::SurfaceCapabilitiesKHR&) const;

    // Debug
    VkDebugUtilsMessengerCreateInfoEXT _debug_info;
    VkDebugUtilsMessengerEXT _debug_messenger;
    void setup_debug();
    void init_debug();
    void destroy_debug();

};

} // namespace tde