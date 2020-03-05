#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <banner/defs.hpp>
#include <banner/gfx/vk_utils.hpp>
#include <banner/gfx/window.hpp>
#include <banner/util/debug.hpp>

namespace bnr {
window::window(str_ref title, vec2 size, str_ref icon_path, bool fullscreen)
    : title_{ title }
    , fullscreen_{ fullscreen }
{
    create_window(title, size);

    if (!icon_path.empty()) {
        set_icon(icon_path);
    }

    if (fullscreen) {
        set_fullscreen(fullscreen);
    }
}

void window::create_window(str_ref title, vec2 size)
{
    title_ = title;

    // TODO: don't call init/terminate from window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    inner_ = glfwCreateWindow(size.x, size.y, title_.c_str(), nullptr, nullptr);

    glfwMakeContextCurrent(inner_);
    glfwSetWindowUserPointer(inner_, this);

    // Event handling
    glfwSetFramebufferSizeCallback(inner_, [](window_inner* wnd, i32 w, i32 h) {
        auto window = to_window(wnd);

        if (!window)
            return;

        window->update_viewport_ = true;
    });
}

window::~window()
{
    glfwDestroyWindow(inner_);
    inner_ = nullptr;
    monitor_ = nullptr;
    // TODO: don't call init/terminate from window
    glfwTerminate();
}

void window::handle_events()
{
    glfwPollEvents();
}

void window::render()
{
    if (!inner_ || is_minimized()) {
        return;
    }

    glfwSwapBuffers(inner_);

    if (update_viewport_) {
        update_viewport_ = false;
        const auto buffer_size = get_framebuffer_size();
        on_resize.fire(u16(buffer_size.x), u16(buffer_size.y));
    }

    on_render.fire();
}

bool window::should_close() const
{
    return glfwWindowShouldClose(inner_);
}

bool window::is_attri_set(int attr) const
{
    return glfwGetWindowAttrib(inner_, attr) == 1;
}

template<int hint>
void window::set_hint(int val)
{
    return glfwWindowHint(hint, val);
}
/**
 * windowed full screen
 */
void window::set_fullscreen(bool status)
{
    if (is_fullscreen() == status)
        return;

    fullscreen_ = status;

    if (status) {
        monitor_ = glfwGetPrimaryMonitor();
        const auto mode = glfwGetVideoMode(monitor_);

        set_hint<GLFW_RED_BITS>(mode->redBits);
        set_hint<GLFW_GREEN_BITS>(mode->greenBits);
        set_hint<GLFW_BLUE_BITS>(mode->blueBits);
        set_hint<GLFW_REFRESH_RATE>(mode->refreshRate);
        set_hint<GLFW_AUTO_ICONIFY>(GLFW_FALSE);

        // switch to full screen
        glfwSetWindowMonitor(inner_, monitor_, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        monitor_ = nullptr;

        // TODO: not hard code reset values
        glfwSetWindowMonitor(inner_, nullptr, 100, 100, 800, 600, 0);
        glfwRestoreWindow(inner_);
    }
}

bool window::is_fullscreen() const
{
    return monitor_ != nullptr && fullscreen_;
}

bool window::is_minimized() const
{
    return is_attri_set(GLFW_ICONIFIED);
}

bool window::is_maximized() const
{
    return is_attri_set(GLFW_MAXIMIZED);
}

void window::set_title(str_ref title)
{
    title_ = title;
    glfwSetWindowTitle(inner_, title_.c_str());
}

void window::set_icon(str_ref filename)
{
    i32 w, h;
    bytes bytes{ stbi_load(filename.c_str(), &w, &h, 0, 4) };
    GLFWimage image{ w, h, (uc8*)bytes };
    glfwSetWindowIcon(inner_, 1, { &image });
    stbi_image_free(image.pixels);
}

void window::set_window_pos(const uv2& size)
{
    glfwSetWindowPos(inner_, (size.x), (size.y));
}

uv2 window::get_window_pos() const
{
    i32 w, h;
    glfwGetWindowPos(inner_, &w, &h);
    return { w, h };
}

void window::set_window_size(const uv2& size)
{
    glfwSetWindowSize(inner_, (size.x), (size.y));
}

uv2 window::get_window_size() const
{
    i32 w, h;
    glfwGetWindowSize(inner_, &w, &h);
    return { w, h };
}

void window::set_mouse_pos(const v2& pos)
{
    glfwSetCursorPos(inner_, pos.x, pos.y);
}

v2 window::get_mouse_pos() const
{
    f64 w, h;
    glfwGetCursorPos(inner_, &w, &h);
    return { w, h };
}

vec2 window::get_framebuffer_size() const
{
    f64 w, h;
    glfwGetFramebufferSize(inner_, reinterpret_cast<int*>(&w), reinterpret_cast<int*>(&h));
    return { w, h };
}

vk::SurfaceKHR window::create_surface(vk::Instance instance) const
{
    VkSurfaceKHR surface{ nullptr };
    VULKAN_CHECK(vk::Result(glfwCreateWindowSurface(instance, inner_, nullptr, &surface)));
    return static_cast<vk::SurfaceKHR>(surface);
}

vector<cstr> window::get_instance_ext() const
{
    vector<cstr> extensions;

    u32 extension_count{ 0 };
    cstr* extension_names;
    extension_names = glfwGetRequiredInstanceExtensions(&extension_count);

    for (auto i{ 0 }; i < extension_count; i++) {
        extensions.push_back(extension_names[i]);
    }

    return extensions;
}

window* window::to_window(GLFWwindow* inner)
{
    return static_cast<window*>(glfwGetWindowUserPointer(inner));
}
} // namespace bnr
