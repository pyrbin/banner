#pragma once

#include "../common/types.hpp"
#include <vector>
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <optional>

using std::vector;

namespace tde {

class platform;

struct queue_family_indices
{
    std::optional<u32> graphics_family;
    std::optional<u32> present_family;

    [[nodiscard]] bool is_same() const
    {
        return has_values() && graphics_family.value() == present_family.value();
    }

    [[nodiscard]] bool has_values() const
    {
        return graphics_family.has_value() && present_family.has_value();
    }
};

struct swapchain_support_details
{
    vk::SurfaceCapabilitiesKHR capabilities;
    vector<vk::SurfaceFormatKHR> formats;
    vector<vk::PresentModeKHR> present_modes;
};

class vulkan_renderer
{
public:
    explicit vulkan_renderer(platform* platform);
    ~vulkan_renderer();
    void draw_frame();

private:
    platform* _platform;

    vk::UniqueInstance _instance;
    vk::UniqueSurfaceKHR _surface;
    vk::PhysicalDevice _physical_device;
    vk::UniqueDevice _device;
    vk::RenderPass _renderpass;

    vk::Queue _graphics_queue;
    vk::Queue _present_queue;

    vk::SwapchainKHR _swapchain;
    vk::SwapchainKHR _old_swapchain {nullptr};

    vk::Extent2D _swap_extent;
    vk::PresentModeKHR _swap_present_mode;
    vk::SurfaceFormatKHR _swap_surface_format;
    vk::Format _depth_buffer_format;
    vector<u32> _queue_family_indices {};
    vk::SharingMode _image_sharing_mode;

    u32 _stage_count {0};
    vector<vk::PipelineShaderStageCreateInfo> _shader_stages {};

    vector<vk::Image> _swapchain_images {};
    vector<vk::ImageView> _swapchain_image_views {};
    vector<vk::Framebuffer> _frame_buffers {};

    vk::UniqueImage _depth_image;
    vk::UniqueImageView _depth_image_view;
    vk::UniqueDeviceMemory _depth_image_memory;

    vk::UniquePipeline _pipeline;
    vk::UniquePipelineLayout _pipeline_layout;

    vk::UniqueCommandPool _command_pool;
    std::vector<vk::CommandBuffer> _command_buffers;

    vector<vk::Semaphore> _image_available_semaphores;
    vector<vk::Semaphore> _render_finished_semaphores;

    vector<vk::Fence> _in_flight_fences;
    vector<vk::Fence> _images_in_flight;

    size_t _current_frame = 0;

    vector<const char*> _device_extensions {};
    vector<const char*> _instance_layers {};
    vector<const char*> _instance_extensions {};

    void get_window_info();
    void init_instance();
    void init_surface();
    void pick_physical_device();
    void create_logical_device();
    void create_swapchain();
    void create_swapchain_images();
    void create_depth_stencil_image();
    void create_render_pass();
    void create_shader();
    void create_graphics_pipeline();
    void create_framebuffers();
    void create_command_buffers();
    void create_sync_objects();

    static vector<char> read_shader_file(const std::string&);
    vk::ShaderModule load_shader_module(const std::string&);
    void destroy_shader_module(vk::ShaderModule) const;

    bool is_device_suitable(const vk::PhysicalDevice&) const;
    bool check_extension_support(const vk::PhysicalDevice&) const;
    queue_family_indices detect_queue_families(const vk::PhysicalDevice&) const;
    swapchain_support_details query_swapchain_details(const vk::PhysicalDevice&) const;

    vk::SurfaceFormatKHR select_swap_format(const std::vector<vk::SurfaceFormatKHR>&) const;
    vk::PresentModeKHR select_swap_mode(const std::vector<vk::PresentModeKHR>&) const;
    vk::Extent2D select_swap_extent(const vk::SurfaceCapabilitiesKHR&) const;

    // Debug
    VkDebugUtilsMessengerCreateInfoEXT _debug_info {};
    VkDebugUtilsMessengerEXT _debug_messenger {};
    void setup_debug();
    void init_debug();
    void destroy_debug();
};

} // namespace tde
