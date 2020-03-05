#include <GLFW/glfw3.h>
#include <algorithm>
#include <memory>
#include <numeric>

#include <banner/defs.hpp>
#include <banner/gfx/graphics.hpp>
#include <banner/gfx/memory.hpp>
#include <banner/gfx/window.hpp>

namespace bnr {

const vector<cstr> graphics::device_extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
const vector<cstr> graphics::validation_layers{ "VK_LAYER_KHRONOS_validation" };

graphics::graphics(window* window)
    : window_{ window }
{

    create_instance();
    create_debugger();
    create_device();

    window_->on_resize.connect<&graphics::resize_swapchain>(*this);
}

graphics::~graphics()
{

    // Destroy debugger
    if (debugger_) {
        const auto destroy = PFN_vkDestroyDebugUtilsMessengerEXT(
            vkGetInstanceProcAddr(instance_.get(), "vkDestroyDebugUtilsMessengerEXT"));
        destroy(instance_.get(), debugger_, nullptr);
    }

    window_->on_resize.disconnect<&graphics::resize_swapchain>(*this);
    window_ = nullptr;
}

void graphics::create_instance()
{
    vk::ApplicationInfo application_info{ "TD Engine", VK_MAKE_VERSION(1, 0, 0), "No engine",
        VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_2 };

    // Instance extensions
    auto instance_extensions{ window_->get_instance_ext() };

    // Debug util extension
    instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    ASSERT(vk_utils::check_instance_extensions(instance_extensions),
        "Required instance extensions not present!");

    ASSERT(vk_utils::check_validation_layers(validation_layers),
        "Required validation layers not present!");

    vk::InstanceCreateInfo instance_info{ vk::InstanceCreateFlags(), &application_info,
        u32(validation_layers.size()), validation_layers.data(), u32(instance_extensions.size()),
        instance_extensions.data() };

    instance_ = vk::createInstanceUnique(instance_info);

    ASSERT(instance_, "Failed to create vulkan instance!");

    debug::trace("Created vulkan instance ...");
}

void graphics::create_surface()
{
    surface_ = vk::UniqueSurfaceKHR(window_->create_surface(instance_.get()), instance_.get());

    ASSERT(surface_, "Failed to create surface!");
}

void graphics::create_device()
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
    swapchain_ =
        std::make_unique<swapchain>(device_.get(), surface_.get(), window_->get_framebuffer_size());

    ASSERT(swapchain_, "Failed to create swapchain!");

    swapchain_->on_recreate.connect([&]() { debug::log("Recreated swapchain"); });

    // Initializing VMA
    memory_ = std::make_unique<memory>(device_.get());

    debug::trace("Initialized vulkan graphics ...");
}

void graphics::reload_swapchain()
{
    auto size = window_->get_framebuffer_size();
    resize_swapchain(size.x, size.y);
}

void graphics::resize_swapchain(u16 w, u16 h)
{
    swapchain_->resize({ w, h });
}

void graphics::create_debugger()
{
    using callback_type = PFN_vkDebugUtilsMessengerCallbackEXT;

    VkDebugUtilsMessengerCreateInfoEXT debug_info;

    debug_info = {};
    debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_info.pfnUserCallback = (callback_type)vk_utils::debug_vulkan_callback;

    const auto create = PFN_vkCreateDebugUtilsMessengerEXT(
        vkGetInstanceProcAddr(instance_.get(), "vkCreateDebugUtilsMessengerEXT"));

    ASSERT(create, "Failed to create debug messenger!");

    create(instance_.get(), (VkDebugUtilsMessengerCreateInfoEXT*)&debug_info, nullptr,
        (VkDebugUtilsMessengerEXT*)&debugger_);

    ASSERT(debugger_, "Failed to create debug messenger");

    debug::trace("Initialized validation layers ...");
}
} // namespace bnr
