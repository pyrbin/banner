#pragma once

#include <optional>
#include <vector>

#include <banner/core/types.hpp>
#include <banner/defs.hpp>
#include <vulkan/vulkan.hpp>

#define VULKAN_CHECK(expr) \
    { \
        A_ASSERT(vk_utils::success(expr)); \
    }

namespace bnr {

struct graphics;

namespace vk_utils {
/**
 * @brief Queue family info
 */
struct queue_family_info
{
    std::optional<u32> graphics_family;
    std::optional<u32> present_family;

    bool is_same() const
    {
        return has_values() && graphics_family.value() == present_family.value();
    }

    bool has_values() const
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
    const vector<vk::SurfaceFormatKHR> formats;
    const vector<vk::PresentModeKHR> present_modes;
};

/**
 *
 */
bool is_device_suitable(const vk::PhysicalDevice device, const vk::SurfaceKHR surface,
    const vector<cstr>& device_extens);

/**
 *
 */
bool check_device_extensions(
    const vk::PhysicalDevice device, const vector<cstr>& device_extens);

/**
 *
 */
bool check_validation_layers(const vector<cstr>& validation_layers);

/**
 *
 */
bool check_instance_extensions(const vector<cstr>& instance_extens);


inline constexpr bool success(VkResult result)
{
    return result == VK_SUCCESS;
}

inline constexpr bool success(vk::Result result)
{
    return result == vk::Result::eSuccess;
}

template<typename T>
inline constexpr bool success(vk::ResultValue<T> result)
{
    return result.result == vk::Result::eSuccess;
}

/**
 *
 */
queue_family_info get_queue_family_info(
    const vk::PhysicalDevice device, const vk::SurfaceKHR surface);

/**
 *
 */
surface_info get_surface_info(
    const vk::PhysicalDevice device, const vk::SurfaceKHR surface);

/**
 *
 */
vk::ShaderModule load_shader(str_ref filename, vk::Device* device);

/**
 *
 */
VKAPI_ATTR VkBool32 VKAPI_CALL debug_vulkan_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* cb_data, void* pUserData);
} // namespace vk_utils
} // namespace bnr
