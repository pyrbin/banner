#pragma once

#include <vector>

#include <banner/core/types.hpp>
#include <banner/gfx/device.hpp>
#include <banner/gfx/memory.hpp>
#include <banner/gfx/swapchain.hpp>
#include <banner/gfx/vk_utils.hpp>
#include <vulkan/vulkan.hpp>

namespace ban {

struct platform;

class graphics
{
public:
    static const std::vector<const char*> validation_layers;
    static const std::vector<const char*> device_extensions;

    graphics(platform* platform);
    ~graphics();

    auto get_swap() { return swapchain_.get(); }
    auto get_device() { return device_.get(); }

private:
    platform* platform_;

    vk::UniqueInstance instance_;
    vk::UniqueSurfaceKHR surface_;

    std::unique_ptr<device> device_;
    std::unique_ptr<swapchain> swapchain_;
    std::unique_ptr<memory> memory_;

    void create_instance();
    void create_surface();
    void create_device();

    void resize_swapchain(u16 w, u16 h);

    vk::DebugUtilsMessengerEXT debugger_;
    void create_debugger();
};
} // namespace ban
