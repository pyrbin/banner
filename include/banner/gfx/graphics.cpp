#include <GLFW/glfw3.h>
#include <algorithm>
#include <memory>
#include <numeric>

#include <banner/core/platform.hpp>
#include <banner/defs.hpp>
#include <banner/gfx/graphics.hpp>
#include <banner/gfx/memory.hpp>

namespace ban {
const std::vector<const char*> graphics::device_extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME };
const std::vector<const char*> graphics::validation_layers{ "VK_LAYER_KHRONOS_validation" };

graphics::graphics(platform* platform)
    : platform_{ platform }
{
    create_instance();
    create_debugger();
    create_device();

    platform_->on_resize.connect<&graphics::resize_swapchain>(this);
}

graphics::~graphics()
{
    // Destroy debugger
    if (debugger_) {
        const auto destroy = PFN_vkDestroyDebugUtilsMessengerEXT(
            vkGetInstanceProcAddr(instance_.get(), "vkDestroyDebugUtilsMessengerEXT"));
        destroy(instance_.get(), debugger_, nullptr);
    }
}

void graphics::create_instance()
{
    // clang-format off
    vk::ApplicationInfo application_info
    {
        "TD Engine",
        VK_MAKE_VERSION(1, 0, 0),
        "No engine",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_2
    };

    // Instance extensions
    auto instance_extensions{ platform_->get_instance_extensions() };

    // Debug util extension
    instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    ASSERT(vk_utils::check_instance_extensions(instance_extensions),
        "Required instance extensions not present!");

    ASSERT(vk_utils::check_validation_layers(validation_layers),
        "Required validation layers not present!");

    // clang-format off
    vk::InstanceCreateInfo instance_info
    {
        vk::InstanceCreateFlags(),
        &application_info,
        u32(validation_layers.size()),
        validation_layers.data(),
        u32(instance_extensions.size()),
        instance_extensions.data()
    };

    instance_ = vk::createInstanceUnique(instance_info);

    ASSERT(instance_, "Failed to create vulkan instance!");

    debug::trace("Created vulkan instance ...");
}

void
graphics::create_surface()
{
    surface_ = vk::UniqueSurfaceKHR(
        platform_->create_surface(instance_.get()), instance_.get());

    ASSERT(surface_, "Failed to create surface!");
}

void
graphics::create_device()
{
    const auto devices = instance_->enumeratePhysicalDevices();

    if (devices.empty()) {
        debug::fatal("No supported physical device were found");
    } else if (devices.size() > 1) {
        debug::err("More than one GPU. This situation is not dealt with yet.");
    }

    // Create our surface;
    create_surface();

    // Make device options
    device::options opts{ device_extensions, validation_layers };

    for (const auto& dev : devices) {
        if (vk_utils::is_device_suitable(dev, surface_.get(), device_extensions)) {
            // Create logical device
            device_ = std::make_unique<device>(dev, surface_.get(), opts);
            break;
        }
    }

    // No suitable devices found
    if (!device_) {
        debug::fatal("No suitable devices found which meet application requirements");
        return;
    }

    // Create swapchain
    swapchain_ = std::make_unique<swapchain>(
        device_.get(), surface_.get(), platform_->get_framebuffer_extent());

    ASSERT(swapchain_, "Failed to create swapchain!");

    swapchain_->on_recreate.connect([&]() { debug::log("Recreated swapchain"); });

    // Initializing VMA
    memory_ = std::make_unique<memory>(device_.get());

    debug::trace("Initialized vulkan graphics ...");
}

void
graphics::resize_swapchain(u16 w, u16 h)
{
    swapchain_->resize({ w, h });
}

void
graphics::create_debugger()
{
    using callback_type = PFN_vkDebugUtilsMessengerCallbackEXT;

    vk::DebugUtilsMessengerCreateInfoEXT debug_info;

    debug_info.setMessageSeverity((vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo));

    debug_info.setMessageType((vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
        | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
        | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation));

    debug_info.setPfnUserCallback((callback_type)vk_utils::debug_vulkan_callback);
    debug_info.setPUserData(this);

    const auto create = PFN_vkCreateDebugUtilsMessengerEXT(
        vkGetInstanceProcAddr(instance_.get(), "vkCreateDebugUtilsMessengerEXT"));

    ASSERT(create, "Failed to create debug messenger!");

    create(instance_.get(), (VkDebugUtilsMessengerCreateInfoEXT*)&debug_info, nullptr,
        (VkDebugUtilsMessengerEXT*)&debugger_);

    debug::trace("Initialized validation layers ...");
}
} // namespace ban
