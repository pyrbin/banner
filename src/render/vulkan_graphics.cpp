#include <unordered_set>
#include <algorithm>
#include <numeric>
#include <GLFW/glfw3.h>

#include "../core/platform.hpp"
#include "../defines.hpp"

#include "vulkan_graphics.hpp"
#include "vulkan_utils.hpp"

namespace tde {

const std::vector<const char*> vulkan_graphics::device_extensions {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const std::vector<const char*> vulkan_graphics::validation_layers {"VK_LAYER_KHRONOS_validation"};

vulkan_graphics::vulkan_graphics(platform* platform) : _platform {platform}
{
    create_instance();
    create_debugger();
    pick_gpu_device();
}

vulkan_graphics::~vulkan_graphics()
{

    if (_debugger) {
        const auto destroy = PFN_vkDestroyDebugUtilsMessengerEXT(
            vkGetInstanceProcAddr(_instance.get(), "vkDestroyDebugUtilsMessengerEXT"));
        destroy(_instance.get(), _debugger, nullptr);
    }
}

void
vulkan_graphics::create_instance()
{
    vk::ApplicationInfo app_info {
        "TD Engine",
        VK_MAKE_VERSION(1, 0, 0),
        "No engine",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_2};

    // Instance extensions
    auto instance_extensions {_platform->get_instance_extensions()};

    // Debug util extension
    instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    ASSERT(
        vulkan_utils::check_instance_extensions(instance_extensions),
        "Required instance extensions not present!");

    ASSERT(
        vulkan_utils::check_validation_layers(validation_layers),
        "Required validation layers not present!");

    _instance = vk::createInstanceUnique(
        {vk::InstanceCreateFlags(),
         &app_info,
         u32(validation_layers.size()),
         validation_layers.data(),
         u32(instance_extensions.size()),
         instance_extensions.data()});

    ASSERT(_instance, "Failed to create vulkan instance!");

    debug::trace("Created vulkan instance ...");
}

void
vulkan_graphics::pick_gpu_device()
{
    const auto devices = _instance->enumeratePhysicalDevices();

    if (devices.empty()) {
        debug::fatal("No supported physical device were found");
    } else if (devices.size() > 1) {
        debug::err("More than one GPU. This situation is not dealt with yet.");
    }

    // Create our surface;
    create_surface();

    for (const auto& device : devices) {
        if (vulkan_utils::is_device_suitable(device, _surface.get(), device_extensions)) {
            // Create logical device
            create_device(device);
            break;
        }
    }

    // No suitable devices found
    if (!_device) {
        debug::fatal("No suitable devices found which meet application requirements");
        return;
    }

    create_swapchain();
    create_imageviews();

    debug::trace("Initialized vulkan graphics ...");
}

void
vulkan_graphics::create_surface()
{
    _surface = vk::UniqueSurfaceKHR(_platform->create_surface(_instance.get()), _instance.get());
    ASSERT(_surface, "Failed to create surface!");
}

void
vulkan_graphics::create_device(const vk::PhysicalDevice device)
{
    std::vector<vk::DeviceQueueCreateInfo> queue_infos;
    const auto indices = vulkan_utils::get_queue_family_info(device, _surface.get());
    const std::unordered_set<u32> unique_family_info {
        indices.graphics_family.value(), indices.present_family.value()};

    f32 priority = 1.0f;
    for (auto idx : unique_family_info) {
        queue_infos.push_back({vk::DeviceQueueCreateFlags(), idx, 1, &priority});
    }

    // Create logical device
    _device = device.createDeviceUnique(
        {vk::DeviceCreateFlags(),
         u32(queue_infos.size()),
         queue_infos.data(),
         u32(validation_layers.size()),
         validation_layers.data(),
         u32(device_extensions.size()),
         device_extensions.data(),
         &device.getFeatures()});

    ASSERT(_device, "Failed to create logical device");

    _gpu = device;

    // TODO: transfer index?
    _graphics_queue = _device->getQueue(indices.graphics_family.value(), 0);
    _present_queue = _device->getQueue(indices.present_family.value(), 0);
}

void
vulkan_graphics::create_swapchain()
{
    _queue_info = vulkan_utils::get_queue_family_info(_gpu, _surface.get());

    auto [capabilities, formats, modes] = vulkan_utils::get_surface_info(_gpu, _surface.get());

    auto surface_format = choose_swap_format(formats);
    auto surface_extent = choose_swap_extent(capabilities);
    auto present_mode = choose_swap_mode(modes);

    u32 image_count =
        std::min<u32>(std::max<u32>(capabilities.minImageCount, 2), capabilities.maxImageCount);

    std::vector<u32> used_indices;
    auto sharing_mode = vk::SharingMode::eExclusive;

    if (!_queue_info.is_same()) {
        used_indices = {_queue_info.graphics_family.value(), _queue_info.present_family.value()};
        sharing_mode = vk::SharingMode::eConcurrent;
    }

    // Create swapchain
    _surface_format = surface_format;
    _swap_extent = surface_extent;

    _swapchain = _device->createSwapchainKHR(
        {vk::SwapchainCreateFlagsKHR(),
         _surface.get(),
         image_count,
         surface_format.format,
         surface_format.colorSpace,
         surface_extent,
         1,
         vk::ImageUsageFlagBits::eColorAttachment,
         sharing_mode,
         u32(used_indices.size()),
         used_indices.data(),
         vk::SurfaceTransformFlagBitsKHR::eIdentity,
         vk::CompositeAlphaFlagBitsKHR::eOpaque,
         present_mode,
         VK_TRUE,
         _old_swapchain});

    ASSERT(_swapchain, "Failed to create swapchain!");
}

void
vulkan_graphics::create_imageviews()
{
    auto [capabilities, formats, modes] = vulkan_utils::get_surface_info(_gpu, _surface.get());

    auto surface_format = choose_swap_format(formats);
    const auto images = _device->getSwapchainImagesKHR(_swapchain);

    _swapchain_imageviews.clear();
    for (const auto& img : images) {
        _swapchain_imageviews.push_back(_device->createImageView({
            vk::ImageViewCreateFlags(),
            img,
            vk::ImageViewType::e2D,
            surface_format.format,
            vk::ComponentMapping(), // R,G,B,A: Identity Components
            vk::ImageSubresourceRange(
                vk::ImageAspectFlagBits::eColor,
                0, // baseMiplevel
                1, // level count
                0, // baseArraylayers
                1) // layers count
        }));
    }
}

vk::SurfaceFormatKHR
vulkan_graphics::choose_swap_format(const std::vector<vk::SurfaceFormatKHR>& formats) const
{
    for (const auto& f : formats) {
        if (f.format == vk::Format::eB8G8R8A8Srgb &&
            f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return f;
        }
    }
    return formats[0];
}

vk::PresentModeKHR
vulkan_graphics::choose_swap_mode(const std::vector<vk::PresentModeKHR>& modes) const
{
    for (const auto& mode : modes) {
        if (mode == vk::PresentModeKHR::eMailbox) {
            return mode;
        }
    }
    return vk::PresentModeKHR::eFifoRelaxed;
}

vk::Extent2D
vulkan_graphics::choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        auto extent = _platform->get_framebuffer_extent();

        extent.width = (std::max)(
            capabilities.minImageExtent.width,
            (std::min)(capabilities.maxImageExtent.width, extent.width));
        extent.height = (std::max)(
            capabilities.minImageExtent.height,
            (std::min)(capabilities.maxImageExtent.height, extent.height));

        return extent;
    }
}

void
vulkan_graphics::create_debugger()
{
    using callback_type = PFN_vkDebugUtilsMessengerCallbackEXT;

    vk::DebugUtilsMessengerCreateInfoEXT debug_info;

    debug_info.setMessageSeverity(
        (vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
             vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
         vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo));

    debug_info.setMessageType(
        (vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
         vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
         vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation));

    debug_info.setPfnUserCallback((callback_type)vulkan_utils::debug_vulkan_callback);
    debug_info.setPUserData(this);

    const auto create = PFN_vkCreateDebugUtilsMessengerEXT(
        vkGetInstanceProcAddr(_instance.get(), "vkCreateDebugUtilsMessengerEXT"));

    ASSERT(create, "Failed to create debug messenger!");

    create(
        _instance.get(),
        (VkDebugUtilsMessengerCreateInfoEXT*)&debug_info,
        nullptr,
        (VkDebugUtilsMessengerEXT*)&_debugger);

    debug::trace("Initialized validation layers ...");
}

} // namespace tde
