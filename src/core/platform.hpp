#pragma once


#include <string>
#include <vulkan/vulkan.hpp>

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
    void get_instance_extensions(u32*, const char***);
    vk::Extent2D get_framebuffer_extent() const;
    void create_surface(vk::Instance*, VkSurfaceKHR*) const;

    GLFWwindow* window() const
    {
        return _window;
    }

private:
    engine* _engine;
    GLFWwindow* _window;
};

}  // namespace tde