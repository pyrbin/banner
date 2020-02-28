#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <banner/util/debug.hpp>
#include <banner/defs.hpp>
#include <banner/core/engine.hpp>
#include <banner/gfx/vk_utils.hpp>
#include <banner/core/platform.hpp>

namespace ban {
platform::platform(const std::string& name)
{
    debug::trace("Initializing platform layer ...");

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window_ = glfwCreateWindow(1280, 720, name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window_, this);

    glfwSetFramebufferSizeCallback(window_, (&on_window_resize));
}

platform::~platform()
{
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }

    glfwTerminate();
}

bool
platform::start_loop()
{
    while (!glfwWindowShouldClose(window_)) {
        update();
    }

    return true;
}

void
platform::update()
{
    glfwPollEvents();
    on_update.fire();
    if (resized_) {
        resized_ = false;

        const auto extent = this->get_framebuffer_extent();

        on_resize.fire(u16(extent.width), u16(extent.height));
    }
}

void
platform::on_window_resize(GLFWwindow* window, int width, int height)
{
    platform* ptr = static_cast<platform*>(glfwGetWindowUserPointer(window));
    ptr->resized_ = true;
}

std::vector<const char*>
platform::get_instance_extensions() const
{
    std::vector<const char*> extensions;

    u32 extension_count{ 0 };
    const char** extension_names;
    extension_names = glfwGetRequiredInstanceExtensions(&extension_count);

    for (auto i{ 0 }; i < extension_count; i++) {
        extensions.push_back(extension_names[i]);
    }

    return extensions;
}

vk::Extent2D
platform::get_framebuffer_extent()
{
    vk::Extent2D extents;
    glfwGetFramebufferSize(window_, reinterpret_cast<int*>(&extents.width),
        reinterpret_cast<int*>(&extents.height));
    return extents;
}

vk::SurfaceKHR
platform::create_surface(vk::Instance instance) const
{
    VkSurfaceKHR surface{ nullptr };
    VULKAN_CHECK(
        vk::Result(glfwCreateWindowSurface(instance, window_, nullptr, &surface)));
    return static_cast<vk::SurfaceKHR>(surface);
}
} // namespace ban