#pragma once

#include <fstream>
#include <vector>

#include "debug.h"
#include "platform.h"
#include "utility.h"
#include "vulkan_render.h"
#include "vulkan_utils.h"
#include <vulkan/vulkan.h>

namespace tde {

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

vulkan_renderer::vulkan_renderer(platform* platform) : _platform{ platform }
{
    debug::trace("Initializing vulkan renderer ...");

    VkApplicationInfo app_info = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    app_info.apiVersion = VK_API_VERSION_1_2;
    app_info.pApplicationName = CFG_ENGINE_NAME;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo instance_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instance_info.pApplicationInfo = &app_info;

    // Extensions.
    const char** pfe = nullptr;
    u32 count = 0;
    _platform->get_required_extensions(&count, &pfe);
    std::vector<const char*> platform_extensions;
    for (u32 i{ 0 }; i < count; ++i) { platform_extensions.push_back(pfe[i]); }

    platform_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    instance_info.enabledExtensionCount = u32(platform_extensions.size());
    instance_info.ppEnabledExtensionNames = platform_extensions.data();

    // Validation layers.
    std::vector<const char*> required_validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    // Get available layers.
    u32 available_layer_count = 0;
    assure_vk(vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr));
    std::vector<VkLayerProperties> available_layers(10);
    assure_vk(vkEnumerateInstanceLayerProperties(&available_layer_count,
                                                 available_layers.data()));
    // Verify all layers are available
    auto success = true;
    for (u32 i{ 0 }; i < u32(required_validation_layers.size()); i++) {
        auto found = false;
        for (u32 j{ 0 }; j < available_layer_count - 1; j++) {
            if (strcmp(required_validation_layers[i], available_layers[j].layerName) ==
                0) {
                found = true;
                break;
            }
        }
        if (!found) {
            success = false;
            debug::fatal("Required validation layer is missing: %s",
                         required_validation_layers[i]);
            break;
        }
    }

    instance_info.enabledLayerCount = u32(required_validation_layers.size());
    instance_info.ppEnabledLayerNames = required_validation_layers.data();

    // Create instance
    assure_vk(vkCreateInstance(&instance_info, nullptr, &_instance));

    // Create debugger
    VkDebugUtilsMessengerCreateInfoEXT debug_info = {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT
    };
    debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_info.pfnUserCallback = debug_callback;
    debug_info.pUserData = this;

    const auto func = PFN_vkCreateDebugUtilsMessengerEXT(
      vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT"));
    ASSERT(func, "Failed to create debug messenger!");
    func(_instance, &debug_info, nullptr, &_debug_messenger);

    // Create surface
    _platform->create_surface(&_instance, &_surface);

    // Select physical device
    _physical_device = select_physical_device();

    // Create logical device
    create_logical_device(required_validation_layers);

    // Shader creation
    create_shader("main");
}

vulkan_renderer::~vulkan_renderer()
{
    if (_debug_messenger) {
        const auto func = PFN_vkDestroyDebugUtilsMessengerEXT(
          vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT"));
        func(_instance, _debug_messenger, nullptr);
    }

    vkDestroyInstance(_instance, nullptr);
}

VkPhysicalDevice
vulkan_renderer::select_physical_device() const
{
    u32 device_count = 0;
    vkEnumeratePhysicalDevices(_instance, &device_count, nullptr);
    if (device_count == 0) { debug::fatal("No supported physical device were found"); }
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(_instance, &device_count, devices.data());

    for (u32 i{ 0 }; i < device_count; i++) {
        if (valid_physical_device(devices[i])) { return devices[i]; }
    }

    debug::fatal("No valid devices found which meet application requirements");

    return nullptr;
}

bool
vulkan_renderer::valid_physical_device(const VkPhysicalDevice physics_device) const
{
    auto gq_idx{ -1 }, pq_idx{ -1 };

    detect_queue_family_indices(physics_device, &gq_idx, &pq_idx);

    auto swapchain_support = query_swapchain_supports(physics_device);
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physics_device, &props);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physics_device, &features);

    auto supports_req_queue_families = (gq_idx != -1) && (pq_idx != -1);

    // Device extension support
    u32 extension_count = 0;
    vkEnumerateDeviceExtensionProperties(
      physics_device, nullptr, &extension_count, nullptr);
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(
      physics_device, nullptr, &extension_count, available_extensions.data());

    // Required extensions
    std::vector<const char*> required_extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    auto success = true;
    for (u64 i = 0; i < required_extensions.size() - 1; i++) {
        auto found = false;
        for (u64 j = 0; j < extension_count - 1; j++) {
            if (strcmp(required_extensions[i], available_extensions[j].extensionName) ==
                0) {
                found = true;
                break;
            }
        }

        if (!found) {
            success = false;
            break;
        }
    }

    auto swapchain_valid = false;
    if (supports_req_queue_families) {
        swapchain_valid = !swapchain_support.formats.empty() &&
                          !swapchain_support.presentation_modes.empty();
    }

    // NOTE: Could also look for discrete GPU. We could score and rank them based on
    // features and capabilities.
    return supports_req_queue_families && swapchain_valid && features.samplerAnisotropy;
}

void
vulkan_renderer::detect_queue_family_indices(VkPhysicalDevice physics_device,
                                             i32* gfx_queue_idx,
                                             i32* presentation_queue_idx) const
{
    u32 queue_family_count{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(
      physics_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> family_props(queue_family_count);

    vkGetPhysicalDeviceQueueFamilyProperties(
      physics_device, &queue_family_count, family_props.data());

    for (u32 i{ 0 }; i < queue_family_count; ++i) {
        if (family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { *gfx_queue_idx = i; }
        VkBool32 supports_presentation = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(
          physics_device, i, _surface, &supports_presentation);
        if (supports_presentation) { *presentation_queue_idx = i; }
    }
}

vulkan_swapchain_support_details
vulkan_renderer::query_swapchain_supports(const VkPhysicalDevice physics_device) const
{
    vulkan_swapchain_support_details details;

    // capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      physics_device, _surface, &details.capabilities);

    // surface formats
    u32 format_count{ 0 };
    vkGetPhysicalDeviceSurfaceFormatsKHR(
      physics_device, _surface, &format_count, nullptr);
    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
          physics_device, _surface, &format_count, details.formats.data());
    }

    // presentation modes
    u32 modes_count{ 0 };
    vkGetPhysicalDeviceSurfaceFormatsKHR(physics_device, _surface, &modes_count, nullptr);
    if (modes_count != 0) {
        details.presentation_modes.resize(modes_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
          physics_device, _surface, &format_count, details.presentation_modes.data());
    }

    return details;
}

void
vulkan_renderer::create_logical_device(
  std::vector<const char*>& required_validation_layers)
{
    auto gq_idx{ -1 }, pq_idx{ -1 };

    detect_queue_family_indices(_physical_device, &gq_idx, &pq_idx);

    // If the queue indices are the same, only one queue needs to be created.
    const auto presentation_shares_gfx_queue = gq_idx == pq_idx;

    std::vector<u32> indices;
    indices.push_back(gq_idx);
    if (!presentation_shares_gfx_queue) { indices.push_back(pq_idx); }

    // Devices queues.
    std::vector<VkDeviceQueueCreateInfo> queue_infos(indices.size());

    for (u32 i{ 0 }; i < u32(queue_infos.size()); i++) {
        queue_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_infos[i].queueFamilyIndex = indices[i];
        queue_infos[i].queueCount = 1;
        queue_infos[i].flags = 0;
        queue_infos[i].pNext = nullptr;

        auto queue_priority = f32(1.0);
        queue_infos[i].pQueuePriorities = &queue_priority;
    }

    VkPhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo device_info{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    device_info.pQueueCreateInfos = queue_infos.data();
    device_info.queueCreateInfoCount = u32(indices.size());
    device_info.pEnabledFeatures = &device_features;
    device_info.enabledExtensionCount = 1;
    const char* required_extensions[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    device_info.ppEnabledExtensionNames = required_extensions;

    // TODO: remove on release
    device_info.enabledLayerCount = u32(required_validation_layers.size());
    device_info.ppEnabledLayerNames = required_validation_layers.data();

    // Create virtual device.
    assure_vk(vkCreateDevice(_physical_device, &device_info, nullptr, &_device));

    _graphics_family_queue_idx = gq_idx;
    _presentation_family_queue_idx = pq_idx;

    // Create the queues.
    vkGetDeviceQueue(_device, _graphics_family_queue_idx, 0, &_graphics_queue);
    vkGetDeviceQueue(_device, _presentation_family_queue_idx, 0, &_presentation_queue);
}

void
vulkan_renderer::create_shader(const char* name)
{
    // vertex shader
    u64 vert_shader_size;
    const auto vertex_shader_source = read_shader_file(name, "vert", &vert_shader_size);
    VkShaderModuleCreateInfo vertex_shader_info = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
    };
    vertex_shader_info.codeSize = vert_shader_size;
    vertex_shader_info.pCode = reinterpret_cast<u32*>(vertex_shader_source);
    VkShaderModule vertex_shader_module;
    assure_vk(
      vkCreateShaderModule(_device, &vertex_shader_info, nullptr, &vertex_shader_module));

    // fragment shader
    u64 frag_shader_size;
    const auto frag_shader_source = read_shader_file(name, "frag", &frag_shader_size);
    VkShaderModuleCreateInfo frag_shader_info = {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
    };
    frag_shader_info.codeSize = frag_shader_size;
    frag_shader_info.pCode = reinterpret_cast<u32*>(frag_shader_source);
    VkShaderModule frag_shader_module;
    assure_vk(
      vkCreateShaderModule(_device, &frag_shader_info, nullptr, &frag_shader_module));

        // Vertex shader stage
    VkPipelineShaderStageCreateInfo vert_shader_stage_info = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
    };
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vertex_shader_module;
    vert_shader_stage_info.pName = "main";

    // Fragment shader stage
    VkPipelineShaderStageCreateInfo frag_shader_stage_info = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
    };
    frag_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    frag_shader_stage_info.module = frag_shader_module;
    frag_shader_stage_info.pName = "main";

    _shader_stage_count = 2;
    _shader_stages.push_back(vert_shader_stage_info);
    _shader_stages.push_back(frag_shader_stage_info);

    delete vertex_shader_source;
    delete frag_shader_source;

}

char*
vulkan_renderer::read_shader_file(const char* filename,
                                  const char* shader_type,
                                  u64* file_size)
{
    char buffer[256];
    u32 length = snprintf(buffer, 256, "shaders/%s.%s.spv", filename, shader_type);
    if (length < 0) { debug::fatal("Shader filename is too long."); }

    buffer[length] = '\0';

    std::ifstream file(buffer, std::ios::ate | std::ios::binary);
    if (!file.is_open()) { debug::fatal("Shader unable to open file."); }

    *file_size = u64(file.tellg());
    const auto file_buffer = static_cast<char*>(malloc(*file_size));
    file.seekg(0);
    file.read(file_buffer, *file_size);
    file.close();

    return file_buffer;
}

} // namespace tde