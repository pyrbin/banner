#pragma once

#include <vector>
#include <optional>
#include <vulkan/vulkan.hpp>

#include "../common/types.hpp"
#include "../debug.hpp"

#define VULKAN_CHECK(expr) \
    { \
        ASSERT(expr == vk::Result::eSuccess); \
    }

namespace tde {
namespace vulkan_utils {

using string_list = std::vector<const char*>;

/**
 * @brief Queue family info
 */
struct queue_family_info
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

/**
 * @brief Surface info
 */
struct surface_info
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;
};

/**
 * @brief Synchronization
 */
struct synchronization
{
    vk::Semaphore lock_render {nullptr};
    vk::Semaphore lock_present {nullptr};
};

/**
 *
 */
bool
check_device_extensions(const vk::PhysicalDevice device, const string_list& device_extens);

/**
 *
 */
bool
check_validation_layers(const string_list& validation_layers);

/**
 *
 */
bool
check_instance_extensions(const string_list& instance_extens);

/**
 *
 */
queue_family_info
get_queue_family_info(const vk::PhysicalDevice device, const vk::SurfaceKHR surface);

/**
 *
 */
surface_info
get_surface_info(const vk::PhysicalDevice device, const vk::SurfaceKHR surface);

/**
 *
 */
vk::ShaderModule
load_shader(const std::string& filename, vk::Device device);

/**
 *
 */
static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_vulkan_callback(
    const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

} // namespace vulkan_utils
} // namespace tde
