#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "debug.h"
#include "Engine.h"
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

const bool
platform::run_loop()
{

    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();
        _engine->tick(0.0);
    }

    return true;
}

void
platform::get_required_extensions(u32* count, const char*** names)
{
    *names = glfwGetRequiredInstanceExtensions(count);
}

} // namespace tde
