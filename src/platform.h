#pragma once

#include <vulkan/vulkan.h>
#include "types.h"
#include <string>

struct GLFWwindow;

namespace tde {

class engine;

class platform
{
public:
    platform(engine* engine, const std::string& name);
    ~platform();

    bool start_loop() const;
    void game_loop() const;

    void get_required_extensions(u32* count, const char*** names);
    void create_surface(VkInstance* instance, VkSurfaceKHR* surface) const;

    GLFWwindow* window() { return _window; }

private:
    engine* _engine;
    GLFWwindow* _window;
};

}