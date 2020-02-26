#include <set>
#include <algorithm>

#include "vulkan_utils.hpp"
#include "../debug.hpp"
#include "../common/utility.hpp"
#include "../common/file.hpp"

namespace tde {
namespace vulkan_utils {

bool
check_device_extensions(const vk::PhysicalDevice device, const string_list& device_extens)
{
    const auto extensions = device.enumerateDeviceExtensionProperties();

    if (extensions.empty())
        return false;

    return std::all_of(device_extens.begin(), device_extens.end(), [&](const auto& name) {
        return std::any_of(extensions.begin(), extensions.end(), [&](const auto& props) {
            return strcmp(name, props.extensionName) == 0;
        });
    });
}

bool
check_validation_layers(const string_list& validation_layers)
{
    auto const layers = vk::enumerateInstanceLayerProperties();

    if (layers.empty())
        return false;

    return std::all_of(validation_layers.begin(), validation_layers.end(), [&](const auto& name) {
        return std::any_of(layers.begin(), layers.end(), [&](const auto& props) {
            return strcmp(name, props.layerName) == 0;
        });
    });
}

bool
check_instance_extensions(const string_list& instance_extens)
{
    const auto extensions = vk::enumerateInstanceExtensionProperties();

    if (extensions.empty())
        return false;

    return std::all_of(instance_extens.begin(), instance_extens.end(), [&](const auto& name) {
        return std::any_of(extensions.begin(), extensions.end(), [&](const auto& props) {
            return strcmp(name, props.extensionName) == 0;
        });
    });
}

queue_family_info
get_queue_family_info(const vk::PhysicalDevice device, const vk::SurfaceKHR surface)
{
    queue_family_info indices {};
    const auto queue_families = device.getQueueFamilyProperties();
    auto i = 0;

    for (const auto& family : queue_families) {
        if (family.queueCount > 0 && family.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphics_family = i;
        }

        const auto present_support = device.getSurfaceSupportKHR(i, VkSurfaceKHR(surface));

        if (family.queueCount > 0 && present_support) {
            indices.present_family = i;
        }

        if (indices.has_values()) {
            break;
        }

        i++;
    }

    return indices;
}

surface_info
get_surface_info(const vk::PhysicalDevice device, const vk::SurfaceKHR surface)
{
    return {
        device.getSurfaceCapabilitiesKHR(surface),
        device.getSurfaceFormatsKHR(surface),
        device.getSurfacePresentModesKHR(surface)};
}

vk::ShaderModule
load_shader(const std::string& filename, vk::Device device)
{
    auto bytes = read_bytes_from_file(filename);
    return device.createShaderModule(
        {vk::ShaderModuleCreateFlags(), bytes.size(), reinterpret_cast<u32*>(bytes.data())});
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_vulkan_callback(
    const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
{
    switch (message_severity) {
    default:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        debug::err(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        debug::warn(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        debug::log(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        debug::trace(callback_data->pMessage);
        break;
    }

    return VK_FALSE;
}

} // namespace vulkan_utils
} // namespace tde
