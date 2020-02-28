#pragma once

#include <string>
#include <functional>

#include <vulkan/vulkan.hpp>
#include <banner/core/types.hpp>
#include <banner/util/signal.hpp>

struct GLFWwindow;

namespace ban {
class engine;

class platform
{
    using window_type = GLFWwindow;

public:
    platform(engine*, const std::string&);
    ~platform();

    bool start_loop();
    void update();

    signal<void(u16, u16)> on_resize;

    std::vector<const char*> get_instance_extensions() const;

    vk::Extent2D get_framebuffer_extent();

    vk::SurfaceKHR create_surface(vk::Instance) const;

    window_type* window() const { return window_; }

private:
    engine* engine_;
    GLFWwindow* window_;

    bool resized_{ false };

    static void on_window_resize(GLFWwindow* window, int width, int height);
};
} // namespace ban