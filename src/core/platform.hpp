#pragma once

#include <string>
#include <vulkan/vulkan.hpp>
#include "../common/types.hpp"

struct GLFWwindow;

namespace tde {

class engine;

class platform
{
public:
    platform(engine*, const std::string&);
    ~platform();

    bool start_loop() const;
    void game_loop() const;

    // Vulkan specific
    std::vector<const char*> get_instance_extensions() const;
    vk::Extent2D get_framebuffer_extent() const;
    vk::SurfaceKHR create_surface(vk::Instance) const;
    GLFWwindow* window() const { return _window; }

private:
    engine* _engine;
    GLFWwindow* _window;
};

} // namespace tde
