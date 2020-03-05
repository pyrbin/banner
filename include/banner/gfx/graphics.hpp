#pragma once

#include <vector>

#include <banner/core/types.hpp>
#include <banner/gfx/device.hpp>
#include <banner/gfx/memory.hpp>
#include <banner/gfx/swapchain.hpp>
#include <banner/gfx/vk_utils.hpp>
#include <vulkan/vulkan.hpp>

namespace bnr {
struct window;

class graphics
{
public:
    static const vector<cstr> validation_layers;
    static const vector<cstr> device_extensions;

    graphics(window* window);
    ~graphics();

    auto get_swap() { return swapchain_.get(); }
    auto get_device() { return device_.get(); }
    vk::ShaderModule load_shader(str_ref filename);
    void reload_swapchain();

private:
    window* window_;

    vk::UniqueInstance instance_;
    vk::UniqueSurfaceKHR surface_;

    uptr<device> device_;
    uptr<swapchain> swapchain_;
    uptr<memory> memory_;

    // Temp storage of shaders
    vector<vk::ShaderModule> shader_modules_;

    void create_instance();
    void create_surface();
    void create_device();

    void resize_swapchain(u16 w, u16 h);

    vk::DebugUtilsMessengerEXT debugger_;
    void create_debugger();
};
} // namespace bnr
