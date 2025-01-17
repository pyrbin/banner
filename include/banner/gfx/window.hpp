#pragma once

#include <string>
#include <vector>

#include <banner/core/types.hpp>
#include <banner/util/signal.hpp>
#include <vulkan/vulkan.hpp>

struct GLFWwindow;
struct GLFWmonitor;

namespace bnr {
struct window
{
    using window_inner = GLFWwindow;
    using window_monitor = GLFWmonitor;

    explicit window(
        str_ref title, vec2 size, str_ref icon_path = "", bool fullscreen = false);
    ~window();

    window_inner* glfw() const { return glfw_; }
    static window* to_window(GLFWwindow* inner);

    auto& title() const { return title_; }
    void set_title(str_ref text);

    void set_icon(str_ref filename);

    uv2 window_size() const;
    void set_window_size(const uv2& size);

    uv2 window_pos() const;
    void set_window_pos(const uv2& size);

    v2 mouse_pos() const;
    void set_mouse_pos(const v2& pos);

    v2 framebuffer_size() const;

    bool is_fullscreen() const;
    void set_fullscreen(bool status = true);

    bool is_minimized() const;
    bool is_maximized() const;

    void render();
    void handle_events();

    bool should_close() const;

    vk::SurfaceKHR create_surface(vk::Instance) const;
    vector<cstr> get_instance_ext() const;

    signal<void(u16, u16)> on_resize;
    signal<void()> on_render;

private:
    void create_window(str_ref title, vec2 size);

    bool is_attri_set(int attr) const;

    template<int hint>
    static void set_hint(int val);

    str title_;

    bool update_viewport_{ false };
    bool fullscreen_{ false };

    window_inner* glfw_{ nullptr };
    window_monitor* monitor_{ nullptr };
};
} // namespace bnr
