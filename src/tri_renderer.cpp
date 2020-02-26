#include "tri_renderer.hpp"
#include "render/vulkan_graphics.hpp"
#include "render/vulkan_utils.hpp"
#include <vulkan/vulkan.hpp>
#include "renderer.hpp"
#include "defines.hpp"
#include "type_traits"

namespace tde {
tri_renderer::tri_renderer(vulkan_graphics* graphics, renderer* renderer)
    : _graphics {graphics}, _renderer {renderer}
{
    _cmd_pool = _graphics->device()->createCommandPoolUnique(
        {vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
         graphics->queue_info()->graphics_family.value()});

    ASSERT(_cmd_pool, "Failed to create cmd pool!");

    _cmd_buffers = _graphics->device()->allocateCommandBuffers(
        {_cmd_pool.get(),
         vk::CommandBufferLevel::ePrimary,
         u32(_graphics->swapchain_imageviews().size())});

    ASSERT(!_cmd_buffers.empty(), "Failed to allocate command buffers");

    createRenderPass();

    createFramebuffers();

    createPipelineLayout();

    createPipeline();
}

void
tri_renderer::update(f32 dt)
{
    u32 index = _renderer->get_index();
    vk::CommandBuffer& command_buffer = _cmd_buffers[index];

    command_buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);

    command_buffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    vk::ClearValue clear_value(
        vk::ClearColorValue {std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f})});

    debug::log("%d", index);

    vk::RenderPassBeginInfo pass_begin_info(
        _render_pass.get(),
        _framebuffers[index],
        vk::Rect2D(vk::Offset2D(0, 0), *_graphics->swapchain_extent()),
        u32(1),
        &clear_value);

    command_buffer.beginRenderPass(&pass_begin_info, vk::SubpassContents::eInline);

    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline.get());

    command_buffer.draw(3, 1, 0, 0);

    command_buffer.endRenderPass();
    command_buffer.end();

    _renderer->submit(command_buffer);
}

void
tri_renderer::createRenderPass()
{
    vk::AttachmentDescription attachment = {};
    attachment.format = *(vk::Format*)_graphics->swapchain_surface_format();
    attachment.samples = vk::SampleCountFlagBits::e1;
    attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
    attachment.storeOp = vk::AttachmentStoreOp::eStore;
    attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachment.initialLayout = vk::ImageLayout::eUndefined;
    attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference ref = {};
    ref.attachment = 0;
    ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ref;

    vk::RenderPassCreateInfo info = {};
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;

    _render_pass = _graphics->device()->createRenderPassUnique(info);
}
void
tri_renderer::createFramebuffers()
{
    const auto buffer_count = _graphics->swapchain_imageviews().size();

    _framebuffers.resize(buffer_count);

    for (auto i = 0; i < buffer_count; i++) {
        std::vector<vk::ImageView> framebuffer_attachments = {_graphics->swapchain_imageviews()[i]};

        _framebuffers[i] =
            _graphics->device()->createFramebuffer({vk::FramebufferCreateFlags(),
                                                    _render_pass.get(),
                                                    u32(framebuffer_attachments.size()),
                                                    framebuffer_attachments.data(),
                                                    u32(_graphics->swapchain_extent()->width),
                                                    u32(_graphics->swapchain_extent()->height),
                                                    u32(1)});

        ASSERT(_framebuffers[i], "Failed to create Framebuffer!");
    }
}

void
tri_renderer::createPipelineLayout()
{
    _pipeline_layout = _graphics->device()->createPipelineLayoutUnique({});
}

void
tri_renderer::createPipeline()
{
    auto vert_shader = vulkan_utils::load_shader("vert.spv", _graphics->device().get());
    auto frag_shader = vulkan_utils::load_shader("frag.spv", _graphics->device().get());

    vk::PipelineShaderStageCreateInfo vert_info = {};
    vert_info.module = vert_shader;
    vert_info.pName = "main";
    vert_info.stage = vk::ShaderStageFlagBits::eVertex;

    vk::PipelineShaderStageCreateInfo frag_info = {};
    frag_info.module = frag_shader;
    frag_info.pName = "main";
    frag_info.stage = vk::ShaderStageFlagBits::eFragment;

    std::vector<vk::PipelineShaderStageCreateInfo> stages {std::move(vert_info),
                                                           std::move(frag_info)};

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;

    vk::Viewport viewport = {};
    viewport.width = static_cast<float>(_graphics->swapchain_extent()->width);
    viewport.height = static_cast<float>(_graphics->swapchain_extent()->height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor = {};
    scissor.extent = *_graphics->swapchain_extent();

    vk::PipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = &viewport;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = &scissor;

    vk::PipelineRasterizationStateCreateInfo rasterizerInfo = {};
    rasterizerInfo.polygonMode = vk::PolygonMode::eFill;
    rasterizerInfo.lineWidth = 1.0f;
    rasterizerInfo.cullMode = vk::CullModeFlagBits::eBack;
    rasterizerInfo.frontFace = vk::FrontFace::eClockwise;

    vk::PipelineMultisampleStateCreateInfo multisampleInfo = {};
    multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampleInfo.minSampleShading = 1.0f;

    vk::PipelineColorBlendAttachmentState colorBlendState = {};
    colorBlendState.colorWriteMask =
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    vk::PipelineColorBlendStateCreateInfo colorBlendInfo = {};
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendState;

    vk::GraphicsPipelineCreateInfo info = {};
    info.stageCount = u32(stages.size());
    info.pStages = stages.data();
    info.pVertexInputState = &vertexInputInfo;
    info.pInputAssemblyState = &inputAssemblyInfo;
    info.pViewportState = &viewportInfo;
    info.pRasterizationState = &rasterizerInfo;
    info.pMultisampleState = &multisampleInfo;
    info.pColorBlendState = &colorBlendInfo;
    info.layout = _pipeline_layout.get();
    info.renderPass = _render_pass.get();
    info.subpass = 0;

    _pipeline = _graphics->device()->createGraphicsPipelineUnique(nullptr, info);

    ASSERT(_pipeline, "Failed to create triangle render pipeline");

    debug::log("Graphics pipeline created!");
}
} // namespace tde