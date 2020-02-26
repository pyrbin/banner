#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "../defines.hpp"
#include "../debug.hpp"

#include "engine.hpp"
#include "platform.hpp"
#include "../render/vulkan_utils.hpp"

namespace tde {

platform::platform(engine* engine, const std::string& name) : _engine {engine}
{
    debug::trace("Initializing platform layer ...");

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    _window = glfwCreateWindow(1280, 720, name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(_window, this);
}

platform::~platform()
{
    if (_window) {
        glfwDestroyWindow(_window);
        _window = nullptr;
    }

    glfwTerminate();
}

bool
platform::start_loop() const
{
    while (!glfwWindowShouldClose(_window)) {
        game_loop();
    }

    return true;
}

void
platform::game_loop() const
{
    glfwPollEvents();
    _engine->tick(0.0);
}

std::vector<const char*>
platform::get_instance_extensions() const
{
    std::vector<const char*> extensions;

    u32 extension_count {0};
    const char** extension_names;
    extension_names = glfwGetRequiredInstanceExtensions(&extension_count);

    for (auto i {0}; i < extension_count; i++) {
        extensions.push_back(extension_names[i]);
    }

    return extensions;
}

vk::Extent2D
platform::get_framebuffer_extent() const
{
    vk::Extent2D extents;
    glfwGetFramebufferSize(
        _window, reinterpret_cast<int*>(&extents.width), reinterpret_cast<int*>(&extents.height));
    return extents;
}

vk::SurfaceKHR
platform::create_surface(vk::Instance instance) const
{
    VkSurfaceKHR surface {nullptr};
    VULKAN_CHECK(vk::Result(glfwCreateWindowSurface(instance, _window, nullptr, &surface)));
    return static_cast<vk::SurfaceKHR>(surface);
}

} // namespace tde
