#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "debug.h"
#include "engine.h"
#include "vulkan_utils.h"
#include "platform.h"

namespace tde {

platform::platform(engine* engine, const std::string& name) : _engine{ engine }
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

void
platform::get_required_extensions(u32* count, const char*** names)
{
    *names = glfwGetRequiredInstanceExtensions(count);
}

void
platform::create_surface(VkInstance* instance, VkSurfaceKHR* surface) const
{
    assure_vk(glfwCreateWindowSurface(*instance, _window, nullptr, surface));
}

} // namespace tde
