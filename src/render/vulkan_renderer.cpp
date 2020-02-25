#pragma once

#include "vulkan_renderer.hpp"

#include <set>
#include <string>
#include <algorithm>

#include "../debug.h"
#include "../platform.h"
#include "../utility.h"
#include "vulkan_utils.hpp"

namespace tde {

vulkan_renderer::vulkan_renderer(platform* platform) : _platform{ platform }
{
    debug::trace("Initializing vulkan renderer ...");

    get_window_info();
    setup_debug();
    init_instance();
    init_debug();
    init_surface();

    pick_physical_device();
    create_logical_device();

    create_swapchain();
    create_swapchain_images();
}

vulkan_renderer::~vulkan_renderer()
{
    _graphics_queue.waitIdle();

    // Destroy device
    _device->destroy();
    _device.release();

    // Destroy surface
    _instance->destroySurfaceKHR(_surface.get());
    _surface.release();

    // Destroy debug
    destroy_debug();

    // Destroy instance
    _instance->destroy();
    _instance.release();
}

void
vulkan_renderer::get_window_info()
{
    u32 extension_count = 0;
    const char** pfe = nullptr;
    _platform->get_instance_extensions(&extension_count, &pfe);

    for (u32 i{ 0 }; i < extension_count; ++i) { _instance_extensions.push_back(pfe[i]); }

    _device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

void
vulkan_renderer::init_instance()
{
    auto app_info = vk::ApplicationInfo("TD Engine",
                                        VK_MAKE_VERSION(1, 0, 0),
                                        "No engine",
                                        VK_MAKE_VERSION(1, 0, 0),
                                        VK_API_VERSION_1_2);

    _instance =
      createInstanceUnique(vk::InstanceCreateInfo(vk::InstanceCreateFlags(),
                                                  &app_info,
                                                  u32(_instance_layers.size()),
                                                  _instance_layers.data(),
                                                  u32(_instance_extensions.size()),
                                                  _instance_extensions.data()));

    ASSERT(_instance, "Failed to create vulkan instance!");
}

void
vulkan_renderer::init_surface()
{
    VkSurfaceKHR psurf = nullptr;
    _platform->create_surface(&_instance.get(), &psurf);
    _surface = vk::UniqueSurfaceKHR(psurf, _instance.get());
}

void
vulkan_renderer::pick_physical_device()
{
    auto devices = _instance->enumeratePhysicalDevices();

    if (devices.empty()) {
        debug::fatal("No supported physical device were found");
    } else if (devices.size() > 1) {
        debug::err("More than one GPU. This situation is not dealt with yet.");
    }

    for (const auto& device : devices) {
        if (is_device_suitable(device)) {
            _physical_device = device;
            return;
        }
    }

    debug::fatal("No valid devices found which meet application requirements");
}

void
vulkan_renderer::create_logical_device()
{
    const auto indices = detect_queue_families(_physical_device);

    const auto same_que_idx = indices.is_same();

    std::vector<u32> unique_que_families;
    unique_que_families.push_back(indices.graphics_family.value());
    if (!same_que_idx) { unique_que_families.push_back(indices.present_family.value()); }

    std::vector<vk::DeviceQueueCreateInfo> queue_infos;
    queue_infos.reserve(unique_que_families.size());

    u32 i{ 0 };
    for (const auto& family_idx : unique_que_families) {
        auto queue_priority = f32(1.0);
        queue_infos.push_back(
          vk::DeviceQueueCreateInfo{ {}, family_idx, 1, &queue_priority });
        i++;
    }

    auto device_features = _physical_device.getFeatures();

    _device = static_cast<vk::UniqueDevice>(_physical_device.createDeviceUnique(
      vk::DeviceCreateInfo(vk::DeviceCreateFlags(),
                           u32(queue_infos.size()),
                           queue_infos.data(),
                           u32(_instance_layers.size()),
                           _instance_layers.data(),
                           u32(_device_extensions.size()),
                           _device_extensions.data(),
                           &device_features)));

    _graphics_queue = _device->getQueue(indices.graphics_family.value(), 0);
    _present_queue = _device->getQueue(indices.present_family.value(), 0);
}

void
vulkan_renderer::create_swapchain()
{
    auto [caps, formats, modes] = query_swapchain_details(_physical_device);

    const auto surface_format = select_swap_format(formats);
    const auto present_mode = select_swap_mode(modes);
    const auto extent = select_swap_extent(caps);

    // Buffer count
    auto image_count = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && image_count > caps.maxImageCount) {
        image_count = caps.maxImageCount;
    }

    const auto indices = detect_queue_families(_physical_device);
    const u32 queue_family_indices[] = { indices.graphics_family.value(),
                                         indices.present_family.value() };

    const auto unique = !indices.is_same();

    // Create swapchain
    _swapchain = _device->createSwapchainKHR(
      vk::SwapchainCreateInfoKHR(
         vk::SwapchainCreateFlagsKHR(),
         _surface.get(),
         image_count,
         surface_format.format,
         surface_format.colorSpace,
         extent,
         1,
         vk::ImageUsageFlagBits::eColorAttachment,
         unique ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
         unique ? 2 : 0,
         unique ? queue_family_indices : nullptr,
         vk::SurfaceTransformFlagBitsKHR::eIdentity,
         vk::CompositeAlphaFlagBitsKHR::eOpaque,
         present_mode,
         VK_TRUE,
         _old_swapchain)
    );

    ASSERT(_swapchain, "Failed to create swapchain!");
       
}

void
vulkan_renderer::create_swapchain_images()
{
    _swapchain_buffers = _device->getSwapchainImagesKHR(_swapchain);
    const auto surface_format = select_swap_format(query_swapchain_details(_physical_device).formats);

    for (const auto& buffer : _swapchain_buffers) {
        _swapchain_buffer_views.push_back(_device->createImageView(vk::ImageViewCreateInfo(
          vk::ImageViewCreateFlags(),
          buffer,
          vk::ImageViewType::e2D,
          surface_format.format,
          vk::ComponentMapping(), // R,G,B,A: Identity Components
          vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor,
                                    0, // baseMiplevel
                                    1, // level count
                                    0, // baseArraylayers
                                    1) // layers count
          )));
    }
}

bool
vulkan_renderer::is_device_suitable(const vk::PhysicalDevice& device) const
{
    const auto indices = detect_queue_families(device);
    const auto supports_ext = check_extension_support(device);

    auto swapchain_valid = false;

    if (supports_ext) {
        const auto swapchain_support = query_swapchain_details(device);
        swapchain_valid =
          !swapchain_support.formats.empty() && !swapchain_support.present_modes.empty();
    }

    return indices.has_values() && supports_ext && swapchain_valid;
}

queue_family_indices
vulkan_renderer::detect_queue_families(const vk::PhysicalDevice& device) const
{
    queue_family_indices indices{};
    const auto queue_families = device.getQueueFamilyProperties();

    auto i = 0;
    for (const auto& family : queue_families) {
        if (family.queueCount > 0 && family.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphics_family = i;
        }

        const auto supports_present =
          device.getSurfaceSupportKHR(i, VkSurfaceKHR(_surface.get()));

        if (family.queueCount > 0 && supports_present) { indices.present_family = i; }

        if (indices.has_values()) { break; }

        i++;
    }

    return indices;
}

bool
vulkan_renderer::check_extension_support(const vk::PhysicalDevice& device) const
{
    auto available_extensions = device.enumerateDeviceExtensionProperties();

    // TODO: not use std::set?
    std::set<std::string> required(_device_extensions.cbegin(),
                                   _device_extensions.cend());

    for (const auto& extension : available_extensions) {
        required.erase(extension.extensionName);
    }

    return required.empty();
}

swapchain_support_details
vulkan_renderer::query_swapchain_details(const vk::PhysicalDevice& device) const
{
    return { device.getSurfaceCapabilitiesKHR(_surface.get()),
             device.getSurfaceFormatsKHR(_surface.get()),
             device.getSurfacePresentModesKHR(_surface.get()) };
}

vk::SurfaceFormatKHR
vulkan_renderer::select_swap_format(
  const std::vector<vk::SurfaceFormatKHR>& formats) const
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
vulkan_renderer::select_swap_mode(const std::vector<vk::PresentModeKHR>& modes) const
{
    for (const auto& mode : modes) {
        if (mode == vk::PresentModeKHR::eMailbox) {
            return mode;
        }
    }
    return vk::PresentModeKHR::eFifoRelaxed;
}

vk::Extent2D
vulkan_renderer::select_swap_extent(const vk::SurfaceCapabilitiesKHR& caps) const
{
    if (caps.currentExtent.width != std::numeric_limits<u32>::max()) {
        return caps.currentExtent;
    }

    auto extent = _platform->get_framebuffer_extent();

    extent.width = static_cast<u32>(extent.width);
    extent.height = static_cast<u32>(extent.height);
    extent.width = std::max(caps.minImageExtent.width,
                            std::min(caps.maxImageExtent.width, extent.width));
    extent.height = std::max(caps.minImageExtent.height,
                             std::min(caps.maxImageExtent.height, extent.height));
    return extent;
}


// VULKAN DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(const VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
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
void
vulkan_renderer::setup_debug()
{
    // Create debugger
    _debug_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    _debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    _debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    _debug_info.pfnUserCallback = debug_callback;
    _debug_info.pUserData = this;

    // Add extension
    _instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    _instance_layers.push_back("VK_LAYER_KHRONOS_validation");
}

void
vulkan_renderer::init_debug()
{
    const auto create = PFN_vkCreateDebugUtilsMessengerEXT(
      vkGetInstanceProcAddr(_instance.get(), "vkCreateDebugUtilsMessengerEXT"));

    ASSERT(create, "Failed to create debug messenger!");

    create(_instance.get(), &_debug_info, nullptr, &_debug_messenger);
}

void
vulkan_renderer::destroy_debug()
{
    if (_debug_messenger) {
        const auto destroy = PFN_vkDestroyDebugUtilsMessengerEXT(
          vkGetInstanceProcAddr(_instance.get(), "vkDestroyDebugUtilsMessengerEXT"));
        destroy(_instance.get(), _debug_messenger, nullptr);
    }
}

} // namespace tde