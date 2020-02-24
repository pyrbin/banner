#pragma once

#include <vector>
#include <fstream>

#include "platform.h"
#include "debug.h"
#include "utility.h"
#include "vulkan_utils.h"
#include "vulkan_render.h"
#include <vulkan/vulkan.h>

namespace tde {

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback (
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
            if (strcmp(required_validation_layers[i], available_layers[j].layerName) == 0) {
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
    debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT, VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_info.pfnUserCallback = debug_callback;
    debug_info.pUserData = this;

    const auto func = PFN_vkCreateDebugUtilsMessengerEXT(
      vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT"));
    ASSERT(func, "Failed to create debug messenger!");
    func(_instance, &debug_info, nullptr, &_debug_messenger);
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

} // namespace tde