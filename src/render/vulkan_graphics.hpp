#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "vulkan_utils.hpp"
#include "../common/types.hpp"

namespace tde {

class platform;

class vulkan_graphics
{
public:
    static const std::vector<const char*> validation_layers;
    static const std::vector<const char*> device_extensions;

    vulkan_graphics(platform* platform);
    ~vulkan_graphics();

    const vk::UniqueDevice& device() const { return _device; }
    const vk::Queue* graphicsQueue() const { return &_graphics_queue; }
    const vk::Queue* presentQueue() const { return &_present_queue; }
    const vulkan_utils::queue_family_info* queue_info() const { return &_queue_info; }
    const vk::SwapchainKHR* swapchain() const { return &_swapchain; }
    const vk::SurfaceFormatKHR* swapchain_surface_format() const { return &_surface_format; }
    const vk::Extent2D* swapchain_extent() const { return &_swap_extent; }

    const std::vector<vk::ImageView>& swapchain_imageviews() const { return _swapchain_imageviews; }

private:
    platform* _platform;

    vk::UniqueInstance _instance;
    vk::UniqueSurfaceKHR _surface;

    vk::PhysicalDevice _gpu;
    vk::UniqueDevice _device;

    vk::Queue _graphics_queue;
    vk::Queue _present_queue;
    vulkan_utils::queue_family_info _queue_info;

    vk::SwapchainKHR _swapchain;
    vk::SurfaceFormatKHR _surface_format;
    vk::Extent2D _swap_extent;
    vk::SwapchainKHR _old_swapchain {nullptr};
    std::vector<vk::ImageView> _swapchain_imageviews;

    void create_instance();
    void pick_gpu_device();
    void create_surface();
    void create_device(const vk::PhysicalDevice);

    void create_swapchain();
    void create_imageviews();
    vk::SurfaceFormatKHR choose_swap_format(const std::vector<vk::SurfaceFormatKHR>&) const;
    vk::PresentModeKHR choose_swap_mode(const std::vector<vk::PresentModeKHR>&) const;
    vk::Extent2D choose_swap_extent(const vk::SurfaceCapabilitiesKHR&) const;

    vk::DebugUtilsMessengerEXT _debugger;
    void create_debugger();
};

} // namespace tde
