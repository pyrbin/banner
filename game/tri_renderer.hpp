#pragma once

#include <vulkan/vulkan.hpp>
#include <banner/gfx.hpp>
#include <banner/util.hpp>

using namespace ban;

struct tri_renderer
{
    using renderer_ptr = ban::renderer*;
    using graphics_ptr = ban::graphics*;

    tri_renderer(renderer_ptr renderer)
        : renderer_{ renderer }

    {
        device_ptr dptr = renderer_->get_swapchain()->get_device();

        vk::CommandPoolCreateInfo info;
        info.setQueueFamilyIndex(dptr->get_queues().graphics_index);
        info.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

        cmd_pool_ = dptr->get().createCommandPoolUnique({});

        vk::CommandBufferAllocateInfo buffer_info;
        buffer_info.setCommandPool(cmd_pool_.get());
        buffer_info.setCommandBufferCount(renderer_->get_swapchain()->get_image_count());

        cmd_buffs_ = dptr->get().allocateCommandBuffers(buffer_info);

        create_render_pass();
        create_frame_buffers();
        create_pipeline_layout();
        create_pipeline();

        // renderer_->set_cmd_buffers(cmd_buffs_.data());
    }

    void update()
    {
        auto idx = renderer_->get_index();
        auto& cmd_buffer = cmd_buffs_[idx];
        cmd_buffer.reset((vk::CommandBufferResetFlagBits)0);

        vk::CommandBufferBeginInfo begin_info;
        begin_info.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        cmd_buffer.begin(begin_info);

        vk::RenderPassBeginInfo render_pass_info;
        render_pass_info.setRenderPass(render_pass_.get());
        render_pass_info.setFramebuffer(framebuffers_[idx]);
        render_pass_info.setRenderArea(
            { vk::Offset2D(0, 0), renderer_->get_swapchain()->get_extent() });

        vk::ClearValue clear_value(
            vk::ClearColorValue{ std::array<float, 4>({ 1.0f, 0.0f, 0.0f, 1.0f }) });

        render_pass_info.setClearValueCount(1);
        render_pass_info.pClearValues = { &clear_value };

        cmd_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);

        cmd_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipe_.get());

        cmd_buffer.draw(3, 1, 0, 0);

        cmd_buffer.endRenderPass();

        cmd_buffer.end();
    }

private:
    renderer_ptr renderer_;
    vk::UniqueCommandPool cmd_pool_;
    std::vector<vk::CommandBuffer> cmd_buffs_;
    vk::UniqueRenderPass render_pass_;
    std::vector<vk::Framebuffer> framebuffers_;
    vk::UniquePipelineLayout pipe_layout_;
    vk::UniquePipeline pipe_;

    void create_render_pass()
    {
        vk::AttachmentDescription attachment;
        attachment.setFormat(renderer_->get_swapchain()->get_format().format);
        attachment.setSamples(vk::SampleCountFlagBits::e1);
        attachment.setLoadOp(vk::AttachmentLoadOp::eDontCare);
        attachment.setStoreOp(vk::AttachmentStoreOp::eStore);
        attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
        attachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
        attachment.setInitialLayout(vk::ImageLayout::eUndefined);
        attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        vk::AttachmentReference ref;
        ref.setAttachment(0);
        ref.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        vk::SubpassDescription subpass;
        subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
        subpass.setColorAttachmentCount(1);
        subpass.setPColorAttachments({ &ref });

        vk::RenderPassCreateInfo info;
        info.setAttachmentCount(1);
        info.setPAttachments({ &attachment });
        info.setSubpassCount(1);
        info.setPSubpasses({ &subpass });

        render_pass_
            = renderer_->get_swapchain()->get_device()->get().createRenderPassUnique(
                info);

        debug::log("Created render pass");
    }

    void create_frame_buffers()
    {
        for (size_t i = 0; i < renderer_->get_swapchain()->get_image_count(); i++) {
            vk::FramebufferCreateInfo info;
            info.setRenderPass(render_pass_.get());
            info.setAttachmentCount(1);
            info.setPAttachments(&renderer_->get_swapchain()->get_data().views[i].get());

            info.setHeight(renderer_->get_swapchain()->get_extent().height);
            info.setWidth(renderer_->get_swapchain()->get_extent().width);
            info.setLayers(1);

            framebuffers_.push_back(
                renderer_->get_swapchain()->get_device()->get().createFramebuffer(info));
        }
    }

    void create_pipeline_layout()
    {
        pipe_layout_
            = renderer_->get_swapchain()->get_device()->get().createPipelineLayoutUnique(
                {});
    }
    void create_pipeline()
    {
        auto vert_shader = vk_utils::load_shader(
            "shaders/shader.vert.spv", renderer_->get_swapchain()->get_device()->get());
        auto frag_shader = vk_utils::load_shader(
            "shaders/shader.frag.spv", renderer_->get_swapchain()->get_device()->get());

        vk::PipelineShaderStageCreateInfo vert_info;
        vert_info.setModule(vert_shader);
        vert_info.setPName("main");
        vert_info.setStage(vk::ShaderStageFlagBits::eVertex);

        vk::PipelineShaderStageCreateInfo frag_info;
        frag_info.setModule(frag_shader);
        frag_info.setPName("main");
        frag_info.setStage(vk::ShaderStageFlagBits::eFragment);

        std::vector<vk::PipelineShaderStageCreateInfo> stages{ std::move(vert_info),
            std::move(frag_info) };

        vk::PipelineVertexInputStateCreateInfo vertex_input_info;
        vk::PipelineInputAssemblyStateCreateInfo input_asm_info = {};
        input_asm_info.setTopology(vk::PrimitiveTopology::eTriangleList);

        vk::Viewport viewport;
        viewport.setWidth(renderer_->get_swapchain()->get_extent().width);
        viewport.setHeight(renderer_->get_swapchain()->get_extent().height);
        viewport.setMinDepth(0.0f);
        viewport.setMaxDepth(1.0f);

        vk::Rect2D scissor = {};
        scissor.setExtent(renderer_->get_swapchain()->get_extent());

        vk::PipelineViewportStateCreateInfo viewport_info;
        viewport_info.setViewportCount(0);
        viewport_info.setPViewports({ &viewport });
        viewport_info.setScissorCount(0);
        viewport_info.setPScissors({ &scissor });

        vk::PipelineRasterizationStateCreateInfo rasterizer_info;
        rasterizer_info.setPolygonMode(vk::PolygonMode::eFill);
        rasterizer_info.setLineWidth(1.0f);
        rasterizer_info.setCullMode(vk::CullModeFlagBits::eBack);
        rasterizer_info.setFrontFace(vk::FrontFace::eCounterClockwise);

        vk::PipelineMultisampleStateCreateInfo multisample_info;
        multisample_info.setRasterizationSamples(vk::SampleCountFlagBits::e1);
        multisample_info.setMinSampleShading(1.0f);

        vk::PipelineColorBlendAttachmentState colorblendstate;
        colorblendstate.setColorWriteMask(vk::ColorComponentFlagBits::eR
            | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB
            | vk::ColorComponentFlagBits::eA);

        vk::PipelineColorBlendStateCreateInfo colorblendinfo;
        colorblendinfo.setAttachmentCount(0);
        colorblendinfo.setPAttachments({ &colorblendstate });

        vk::GraphicsPipelineCreateInfo info;

        info.setStageCount(u32(stages.size()));
        info.setPStages(stages.data());

        info.setPVertexInputState(&vertex_input_info);
        info.setPInputAssemblyState(&input_asm_info);
        info.setPViewportState(&viewport_info);
        info.setPMultisampleState(&multisample_info);
        info.setPColorBlendState(&colorblendinfo);
        info.setLayout(pipe_layout_.get());
        info.setRenderPass(render_pass_.get());
        info.setSubpass(0);

        pipe_ = renderer_->get_swapchain()
                    ->get_device()
                    ->get()
                    .createGraphicsPipelineUnique(nullptr, info);

        ASSERT(pipe_, "Failed to create pipe");

        debug::trace("Created pipeline");
    }
};
