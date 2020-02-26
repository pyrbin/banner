#pragma once

#include "vulkan_renderer.hpp"

#include <algorithm>
#include <fstream>
#include <set>
#include <string>

#include "../debug.hpp"
#include "../debug.hpp"
#include "../core/platform.hpp"
#include "../common/utility.hpp"
#include "vulkan_utils.hpp"

const int MAX_FRAMES_IN_FLIGHT = 2;

namespace tde {

vulkan_renderer::vulkan_renderer(platform* platform) : _platform {platform}
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
    create_depth_stencil_image();
    create_render_pass();
    create_graphics_pipeline();
    create_framebuffers();
    create_command_buffers();
    create_sync_objects();
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

    for (u32 i {0}; i < extension_count; ++i) {
        _instance_extensions.push_back(pfe[i]);
    }

    _device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

void
vulkan_renderer::init_instance()
{
    auto app_info = vk::ApplicationInfo(
        "TD Engine",
        VK_MAKE_VERSION(1, 0, 0),
        "No engine",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_2);

    _instance = vk::createInstanceUnique(
        {vk::InstanceCreateFlags(),
         &app_info,
         u32(_instance_layers.size()),
         _instance_layers.data(),
         u32(_instance_extensions.size()),
         _instance_extensions.data()});

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
    if (!same_que_idx) {
        unique_que_families.push_back(indices.present_family.value());
    }

    std::vector<vk::DeviceQueueCreateInfo> queue_infos;
    queue_infos.reserve(unique_que_families.size());

    u32 i {0};
    for (const auto& family_idx : unique_que_families) {
        auto queue_priority = f32(1.0);
        queue_infos.push_back(vk::DeviceQueueCreateInfo {{}, family_idx, 1, &queue_priority});
        i++;
    }

    auto device_features = _physical_device.getFeatures();

    _device = static_cast<vk::UniqueDevice>(_physical_device.createDeviceUnique(
        {vk::DeviceCreateFlags(),
         u32(queue_infos.size()),
         queue_infos.data(),
         u32(_instance_layers.size()),
         _instance_layers.data(),
         u32(_device_extensions.size()),
         _device_extensions.data(),
         &device_features}));

    _graphics_queue = _device->getQueue(indices.graphics_family.value(), 0);
    _present_queue = _device->getQueue(indices.present_family.value(), 0);
}

void
vulkan_renderer::create_swapchain()
{
    auto [caps, formats, modes] = query_swapchain_details(_physical_device);

    _swap_surface_format = select_swap_format(formats);
    _swap_extent = select_swap_extent(caps);
    _swap_present_mode = select_swap_mode(modes);

    // Buffer count
    auto image_count = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && image_count > caps.maxImageCount) {
        image_count = caps.maxImageCount;
    }

    const auto indices = detect_queue_families(_physical_device);

    if (!indices.is_same()) {
        _queue_family_indices = {indices.graphics_family.value(), indices.present_family.value()};
        _image_sharing_mode = vk::SharingMode::eConcurrent;
    } else {
        _image_sharing_mode = vk::SharingMode::eExclusive;
    }

    // Create swapchain
    _swapchain = _device->createSwapchainKHR(
        {vk::SwapchainCreateFlagsKHR(),
         _surface.get(),
         image_count,
         _swap_surface_format.format,
         _swap_surface_format.colorSpace,
         _swap_extent,
         1,
         vk::ImageUsageFlagBits::eColorAttachment,
         _image_sharing_mode,
         u32(_queue_family_indices.size()),
         _queue_family_indices.data(),
         vk::SurfaceTransformFlagBitsKHR::eIdentity,
         vk::CompositeAlphaFlagBitsKHR::eOpaque,
         _swap_present_mode,
         VK_TRUE,
         _old_swapchain});

    ASSERT(_swapchain, "Failed to create swapchain!");
}

void
vulkan_renderer::create_swapchain_images()
{
    _swapchain_images = _device->getSwapchainImagesKHR(_swapchain);
    for (const auto& img : _swapchain_images) {
        _swapchain_image_views.push_back(_device->createImageView({
            vk::ImageViewCreateFlags(),
            img,
            vk::ImageViewType::e2D,
            _swap_surface_format.format,
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

void
vulkan_renderer::create_depth_stencil_image()
{
    // Check for the format of the Depth/Stencil Buffer
    vector<vk::Format> depth_formats {
        vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat, vk::Format::eD24UnormS8Uint};

    for (auto format : depth_formats) {
        auto format_properties = _physical_device.getFormatProperties(format);
        if (format_properties.optimalTilingFeatures &
            vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
            _depth_buffer_format = format;
            break;
        }
    }

    if (_depth_buffer_format == vk::Format::eUndefined)
        debug::fatal("Unable to find a supported depth format");

    _depth_image = _device->createImageUnique(
        {vk::ImageCreateFlags(),
         vk::ImageType::e2D,
         _depth_buffer_format,
         vk::Extent3D(_swap_extent, 1),
         1,
         1,
         vk::SampleCountFlagBits::e1,
         vk::ImageTiling::eOptimal,
         vk::ImageUsageFlagBits::eDepthStencilAttachment,
         _image_sharing_mode,
         u32(_queue_family_indices.size()),
         _queue_family_indices.data(),
         vk::ImageLayout::eUndefined});

    // Get the required information about memory for the image
    auto memory_req {_device->getImageMemoryRequirements(_depth_image.get())};
    auto memory_index {std::numeric_limits<u32>::max()};
    // Application controlled memory property (GPU local memory)
    auto memory_props {vk::MemoryPropertyFlagBits::eDeviceLocal};

    auto physical_mem_props {_physical_device.getMemoryProperties()};
    for (u32 i = 0; i < physical_mem_props.memoryTypeCount; i++) {
        if (memory_req.memoryTypeBits & (1 << i)) {
            if ((physical_mem_props.memoryTypes[i].propertyFlags & memory_props) == memory_props) {
                memory_index = i;
                break;
            }
        }
    }

    ASSERT(memory_index != std::numeric_limits<u32>::max());

    _depth_image_memory = _device->allocateMemoryUnique({memory_req.size, memory_index});
    _device->bindImageMemory(_depth_image.get(), _depth_image_memory.get(), 0);

    _depth_image_view = _device->createImageViewUnique(
        {vk::ImageViewCreateFlags(),
         _depth_image.get(),
         vk::ImageViewType::e2D,
         _depth_buffer_format,
         vk::ComponentMapping(),
         vk::ImageSubresourceRange(
             vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1)});
}

void
vulkan_renderer::create_render_pass()
{
    // Render pass attachment descriptions
    vector<vk::AttachmentDescription> attachments {
        // Depth buffer attachment
        {vk::AttachmentDescriptionFlags(),
         _depth_buffer_format,
         vk::SampleCountFlagBits::e1,
         vk::AttachmentLoadOp::eClear,
         vk::AttachmentStoreOp::eDontCare,
         vk::AttachmentLoadOp::eDontCare,
         vk::AttachmentStoreOp::eStore,
         vk::ImageLayout::eUndefined,
         vk::ImageLayout::eDepthStencilAttachmentOptimal},
        // Stencil buffer / color attachment
        {vk::AttachmentDescriptionFlags(),
         _swap_surface_format.format,
         vk::SampleCountFlagBits::e1,
         vk::AttachmentLoadOp::eClear,
         vk::AttachmentStoreOp::eStore,
         vk::AttachmentLoadOp::eDontCare,
         vk::AttachmentStoreOp::eDontCare,
         vk::ImageLayout::eUndefined,
         vk::ImageLayout::ePresentSrcKHR},
        // additional attachments
    };

    vector<vk::AttachmentReference> depth_reference {
        vk::AttachmentReference(0, vk::ImageLayout::eDepthStencilAttachmentOptimal)};

    vector<vk::AttachmentReference> stencil_reference {
        vk::AttachmentReference(1, vk::ImageLayout::eColorAttachmentOptimal)
        // additional color attachments
    };

    // Sub-passes used for stages that are part of the renderpass
    vector<vk::SubpassDescription> subpasses = {
        vk::SubpassDescription {
            vk::SubpassDescriptionFlags(),
            vk::PipelineBindPoint::eGraphics,
            0,
            nullptr,
            u32(stencil_reference.size()),
            stencil_reference.data(),
            nullptr,
            depth_reference.data(),
            0,
            nullptr},
        // additional sub-passes
    };

    // Sub-pass dependencies (for extra control)

    vector<vk::SubpassDependency> dependency {

        {VK_SUBPASS_EXTERNAL,
         0,
         vk::PipelineStageFlagBits::eColorAttachmentOutput,
         vk::PipelineStageFlagBits::eColorAttachmentOutput,
         vk::AccessFlagBits(0),
         vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
         vk::DependencyFlags()}};

    // The renderpass where rendering operations are performed
    _renderpass = _device->createRenderPass(
        {vk::RenderPassCreateFlags(),
         u32(attachments.size()),
         attachments.data(),
         u32(subpasses.size()),
         subpasses.data(),
         u32(dependency.size()),
         dependency.data()});
}

void
vulkan_renderer::create_shader()
{
    auto vert_shader_module = load_shader_module("vert.spv");
    auto frag_shader_module = load_shader_module("frag.spv");

    // Vertex shader stage
    const vk::PipelineShaderStageCreateInfo vert_info {
        vk::PipelineShaderStageCreateFlags(),
        vk::ShaderStageFlagBits::eVertex,
        vert_shader_module,
        "main"};

    // Fragment shader stage
    const vk::PipelineShaderStageCreateInfo frag_info {
        vk::PipelineShaderStageCreateFlags(),
        vk::ShaderStageFlagBits::eFragment,
        frag_shader_module,
        "main"};
    _stage_count = 2;
    _shader_stages = {vert_info, frag_info};

    // delete vert_shader_module;
    // delete frag_shader_module;
}

void
vulkan_renderer::create_graphics_pipeline()
{
    create_shader();

    // Viewport
    // Swaps rendering to bottom-to-top (open-gl style)
    auto viewport = vk::Viewport {
        f32(0),
        f32(_swap_extent.height),
        f32(_swap_extent.width),
        -f32(_swap_extent.width),
        f32(0),
        f32(1)};

    // Scissor
    vk::Rect2D scissor {{0, 0}, {_swap_extent.width, _swap_extent.height}};

    // Viewport state
    vk::PipelineViewportStateCreateInfo viewport_info {
        vk::PipelineViewportStateCreateFlags(), 1, &viewport, 1, &scissor};

    // Rasterizer
    vk::PipelineRasterizationStateCreateInfo rasterizer_info {
        vk::PipelineRasterizationStateCreateFlags(),
        VK_FALSE,
        VK_FALSE,
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        // Counter-clockwise because we flipped the view on y-axis
        vk::FrontFace::eCounterClockwise,
        VK_FALSE,
        f32(0),
        f32(0),
        f32(0),
        f32(1)};

    // Multisampling
    vk::PipelineMultisampleStateCreateInfo multisampling_info {
        vk::PipelineMultisampleStateCreateFlags(),
        vk::SampleCountFlagBits::e1,
        VK_FALSE,
        f32(1),
        nullptr,
        VK_FALSE,
        VK_FALSE

    };

    // Depth and stencil testing.
    vk::PipelineDepthStencilStateCreateInfo depth_stencil_info {
        vk::PipelineDepthStencilStateCreateFlags(),
        VK_TRUE,
        VK_TRUE,
        vk::CompareOp::eLess,
        VK_FALSE,
        VK_FALSE};

    vk::PipelineColorBlendAttachmentState color_blend_attachment {
        VK_FALSE,
    };
    color_blend_attachment.setColorWriteMask(
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    vk::PipelineColorBlendStateCreateInfo color_blend_info {
        vk::PipelineColorBlendStateCreateFlags(),
        VK_FALSE,
        vk::LogicOp::eCopy,
        1,
        &color_blend_attachment,
        {0.f, 0.f, 0.f, 0.f}};

    // Dynamic state
    vk::DynamicState dynamic_states[] {vk::DynamicState::eViewport, vk::DynamicState::eLineWidth};

    vk::PipelineDynamicStateCreateInfo dynamic_state_info {
        vk::PipelineDynamicStateCreateFlags(), 2, dynamic_states};

    vk::PipelineVertexInputStateCreateInfo vertex_input_info {
        vk::PipelineVertexInputStateCreateFlags(), 0, 0};

    // Input assembly
    vk::PipelineInputAssemblyStateCreateInfo input_assembly {
        vk::PipelineInputAssemblyStateCreateFlags(),
        vk::PrimitiveTopology::eTriangleList,
        VK_FALSE};

    // Pipeline layout
    _pipeline_layout = _device->createPipelineLayoutUnique({
        vk::PipelineLayoutCreateFlags(),
        0,
        nullptr,
        0,
        nullptr,
    });

    // Create pipeline
    _pipeline = _device->createGraphicsPipelineUnique(
        nullptr,
        {vk::PipelineCreateFlags(),
         _stage_count,
         _shader_stages.data(),
         &vertex_input_info,
         &input_assembly,
         nullptr,
         &viewport_info,
         &rasterizer_info,
         &multisampling_info,
         &depth_stencil_info,
         &color_blend_info,
         nullptr,
         _pipeline_layout.get(),
         _renderpass,
         0,
         nullptr,
         -1});

    debug::log("Graphics pipeline created!");
}

void
vulkan_renderer::create_framebuffers()
{
    const auto buffer_count = _swapchain_image_views.size();
    _frame_buffers.resize(buffer_count);
    for (auto i = 0; i < buffer_count; i++) {
        vector<vk::ImageView> framebuffer_attachments = {
            _depth_image_view.get(), _swapchain_image_views[i]};

        _frame_buffers[i] = _device->createFramebuffer(
            {vk::FramebufferCreateFlags(),
             _renderpass,
             u32(framebuffer_attachments.size()),
             framebuffer_attachments.data(),
             _swap_extent.width,
             _swap_extent.height,
             1});
        ASSERT(_frame_buffers[i], "Failed to create Framebuffer!");
    }
}

void
vulkan_renderer::create_command_buffers()
{
    // Command pool
    auto indices {detect_queue_families(_physical_device)};
    _command_pool = _device->createCommandPoolUnique(
        {vk::CommandPoolCreateFlags(), indices.graphics_family.value()});
    ASSERT(_command_pool, "Failed to create Command pool!");

    // Command buffers
    _command_buffers = _device->allocateCommandBuffers(
        {_command_pool.get(),
         vk::CommandBufferLevel::ePrimary,
         u32(_swapchain_image_views.size())});
    ASSERT(!_command_buffers.empty(), "Failed to allocate command buffers");

    for (u32 i = 0; i < _command_buffers.size(); i++) {
        vk::CommandBufferBeginInfo begin_info(vk::CommandBufferUsageFlags(), nullptr);
        _command_buffers[i].begin(begin_info);

        vk::ClearValue clear_value(
            vk::ClearColorValue {std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f})});

        vk::RenderPassBeginInfo pass_begin_info(
            _renderpass,
            _frame_buffers[i],
            vk::Rect2D(vk::Offset2D(0, 0), _swap_extent),
            u32(2),
            &clear_value);

        _command_buffers[i].beginRenderPass(&pass_begin_info, vk::SubpassContents::eInline);
        _command_buffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline.get());
        _command_buffers[i].draw(3, 1, 0, 0);
        _command_buffers[i].endRenderPass();
        _command_buffers[i].end();
    }
}

void
vulkan_renderer::create_sync_objects()
{
    _image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
    _images_in_flight.resize(_swapchain_images.size(), nullptr);

    vk::SemaphoreCreateInfo semph_info {};
    vk::FenceCreateInfo fence_info {vk::FenceCreateFlagBits::eSignaled};
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        _device->createSemaphore(&semph_info, nullptr, &_image_available_semaphores[i]);
        _device->createSemaphore(&semph_info, nullptr, &_render_finished_semaphores[i]);
        _device->createFence(&fence_info, nullptr, &_in_flight_fences[i]);
    }
}

void
vulkan_renderer::draw_frame()
{
    _device->waitForFences(1, &_in_flight_fences[_current_frame], VK_TRUE, UINT64_MAX);

    const auto image_index = _device->acquireNextImageKHR(
        _swapchain, UINT64_MAX, _image_available_semaphores[_current_frame], nullptr);

    if (_images_in_flight[image_index.value]) {
        _device->waitForFences(1, &_images_in_flight[image_index.value], VK_TRUE, UINT64_MAX);
    }

    _images_in_flight[image_index.value] = _in_flight_fences[_current_frame];

    vk::Semaphore wait_semaphores[] {_image_available_semaphores[_current_frame]};
    vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::Semaphore signal_semaphores[] = {_render_finished_semaphores[_current_frame]};

    vk::SubmitInfo submit_info(
        1,
        wait_semaphores,
        wait_stages,
        1,
        &_command_buffers[image_index.value],
        1,
        signal_semaphores);
    _device->resetFences(1, &_in_flight_fences[_current_frame]);

    auto res = _graphics_queue.submit(1, &submit_info, _in_flight_fences[_current_frame]);

    assert_vk_success(res);

    vk::SwapchainKHR swapchains[] = {_swapchain};
    vk::PresentInfoKHR present_info(1, signal_semaphores, 1, swapchains, &image_index.value);

    assert_vk_success(_present_queue.presentKHR(&present_info));

    _current_frame = (_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

vk::ShaderModule
vulkan_renderer::load_shader_module(const std::string& filename)
{
    auto bin = read_shader_file(filename);
    return _device->createShaderModule(
        {vk::ShaderModuleCreateFlags(), bin.size(), reinterpret_cast<u32*>(bin.data())});
}

void
vulkan_renderer::destroy_shader_module(vk::ShaderModule shader) const
{
    _device->destroyShaderModule(shader);
    delete shader;
}

vector<char>
vulkan_renderer::read_shader_file(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        debug::err("Failed to read shader file: %s", filename);
    }
    const size_t file_size = file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), file_size);
    file.close();
    return buffer;
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
    queue_family_indices indices {};
    const auto queue_families = device.getQueueFamilyProperties();

    auto i = 0;
    for (const auto& family : queue_families) {
        if (family.queueCount > 0 && family.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphics_family = i;
        }

        const auto supports_present = device.getSurfaceSupportKHR(i, VkSurfaceKHR(_surface.get()));

        if (family.queueCount > 0 && supports_present) {
            indices.present_family = i;
        }

        if (indices.has_values()) {
            break;
        }

        i++;
    }

    return indices;
}

bool
vulkan_renderer::check_extension_support(const vk::PhysicalDevice& device) const
{
    auto available_extensions = device.enumerateDeviceExtensionProperties();

    // TODO: not use std::set?
    std::set<std::string> required(_device_extensions.cbegin(), _device_extensions.cend());

    for (const auto& extension : available_extensions) {
        required.erase(extension.extensionName);
    }

    return required.empty();
}

swapchain_support_details
vulkan_renderer::query_swapchain_details(const vk::PhysicalDevice& device) const
{
    return {
        device.getSurfaceCapabilitiesKHR(_surface.get()),
        device.getSurfaceFormatsKHR(_surface.get()),
        device.getSurfacePresentModesKHR(_surface.get())};
}

vk::SurfaceFormatKHR
vulkan_renderer::select_swap_format(const std::vector<vk::SurfaceFormatKHR>& formats) const
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
    extent.width =
        std::max(caps.minImageExtent.width, std::min(caps.maxImageExtent.width, extent.width));
    extent.height =
        std::max(caps.minImageExtent.height, std::min(caps.maxImageExtent.height, extent.height));
    return extent;
}

// VULKAN DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(
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
void
vulkan_renderer::setup_debug()
{
    // Create debugger
    _debug_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
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
