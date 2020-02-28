#pragma once

#include <vector>
#include <optional>
#include <vulkan/vulkan.hpp>

#include <banner/core/types.hpp>
#include <banner/defs.hpp>

#define VULKAN_CHECK(expr) \
    { \
        A_ASSERT(expr == vk::Result::eSuccess); \
    }

namespace ban {
namespace vk_utils {

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
    const vk::SurfaceCapabilitiesKHR capabilities;
    const std::vector<vk::SurfaceFormatKHR> formats;
    const std::vector<vk::PresentModeKHR> present_modes;
};

/**
 * @brief Synchronization
 */
struct synchronization
{
    vk::Semaphore lock_render{ nullptr };
    vk::Semaphore lock_present{ nullptr };
};

/**
 * @brief Synchronization
 */
struct vulkan_gpu
{
    // Device info
};

/**
 *
 */
bool
is_device_suitable(
    const vk::PhysicalDevice device,
    const vk::SurfaceKHR surface,
    const std::vector<const char*>& device_extens);

/**
 *
 */
bool
check_device_extensions(const vk::PhysicalDevice device, const std::vector<const char*>& device_extens);

/**
 *
 */
bool
check_validation_layers(const std::vector<const char*>& validation_layers);

/**
 *
 */
bool
check_instance_extensions(const std::vector<const char*>& instance_extens);

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
VKAPI_ATTR vk::Bool32 VKAPI_CALL
debug_vulkan_callback(
    const vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
    vk::DebugUtilsMessageTypeFlagBitsEXT message_types,
    const vk::DebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

} // namespace vk_utils
} // namespace ban
