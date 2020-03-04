#include <algorithm>
#include <set>

#include <banner/defs.hpp>
#include <banner/gfx/vk_utils.hpp>
#include <banner/util/file.hpp>

namespace ban {
namespace vk_utils {
bool is_device_suitable(const vk::PhysicalDevice device, const vk::SurfaceKHR surface,
    const std::vector<const char*>& device_extens)
{
    const auto indices = get_queue_family_info(device, surface);
    const auto supports_ext = check_device_extensions(device, device_extens);

    auto swapchain_valid = false;

    if (supports_ext) {
        const auto swapchain_support = get_surface_info(device, surface);
        swapchain_valid =
            !swapchain_support.formats.empty() && !swapchain_support.present_modes.empty();
    }

    return indices.has_values() && supports_ext && swapchain_valid;
}

bool check_device_extensions(
    const vk::PhysicalDevice device, const std::vector<const char*>& device_extens)
{
    const auto extensions = device.enumerateDeviceExtensionProperties();

    if (extensions.empty())
        return false;

    return std::all_of(device_extens.begin(), device_extens.end(), [&](const auto& name) {
        return std::any_of(extensions.begin(), extensions.end(),
            [&](const auto& props) { return strcmp(name, props.extensionName) == 0; });
    });
}

bool check_validation_layers(const std::vector<const char*>& validation_layers)
{
    auto const layers = vk::enumerateInstanceLayerProperties();

    if (layers.empty())
        return false;

    return std::all_of(validation_layers.begin(), validation_layers.end(), [&](const auto& name) {
        return std::any_of(layers.begin(), layers.end(),
            [&](const auto& props) { return strcmp(name, props.layerName) == 0; });
    });
}

bool check_instance_extensions(const std::vector<const char*>& instance_extens)
{
    const auto extensions = vk::enumerateInstanceExtensionProperties();

    if (extensions.empty())
        return false;

    return std::all_of(instance_extens.begin(), instance_extens.end(), [&](const auto& name) {
        return std::any_of(extensions.begin(), extensions.end(),
            [&](const auto& props) { return strcmp(name, props.extensionName) == 0; });
    });
}

queue_family_info get_queue_family_info(
    const vk::PhysicalDevice device, const vk::SurfaceKHR surface)
{
    queue_family_info indices{};
    const auto queue_families = device.getQueueFamilyProperties();
    u32 i = 0;

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

surface_info get_surface_info(const vk::PhysicalDevice device, const vk::SurfaceKHR surface)
{
    return { device.getSurfaceCapabilitiesKHR(surface), device.getSurfaceFormatsKHR(surface),
        device.getSurfacePresentModesKHR(surface) };
}

vk::ShaderModule load_shader(const std::string& filename, vk::Device device)
{
    auto bytes = read_bytes_from_file(filename);
    return device.createShaderModule(
        { vk::ShaderModuleCreateFlags(), bytes.size(), reinterpret_cast<u32*>(bytes.data()) });
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL debug_vulkan_callback(
    const vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity,
    vk::DebugUtilsMessageTypeFlagBitsEXT message_types,
    const vk::DebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data)
{
    switch (message_severity) {
    default:
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
        debug::err(callback_data->pMessage);
        break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
        debug::warn(callback_data->pMessage);
        break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
        debug::log(callback_data->pMessage);
        break;
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
        debug::trace(callback_data->pMessage);
        break;
    }

    return VK_FALSE;
}
} // namespace vk_utils
} // namespace ban
