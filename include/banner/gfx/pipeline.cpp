#include <banner/core/types.hpp>
#include <banner/gfx/pipeline.hpp>
#include <banner/gfx/swapchain.hpp>
#include <banner/util/debug.hpp>

namespace ban {

void pipeline::build(swapchain::ptr swap, vk::RenderPass render_pass)
{
    auto device = swap->get_device();
    auto extent = swap->get_extent();


    // Viewport
    // Swaps rendering to bottom-to-top (open-gl style)
    auto viewport = vk::Viewport{ f32(0), f32(extent.height), f32(extent.width), -f32(extent.width),
        f32(0), f32(1) };
    // Scissor
    auto scissor = vk::Rect2D{ { 0, 0 }, { extent.width, extent.height } };

    // Viewport state
    auto viewport_info =
        vk::PipelineViewportStateCreateInfo{ vk::PipelineViewportStateCreateFlags(), 1, &viewport,
            1, &scissor };

    // Rasterizer
    auto rasterizer_info =
        vk::PipelineRasterizationStateCreateInfo{ vk::PipelineRasterizationStateCreateFlags(),
            VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
            // Counter-clockwise because we flipped the view on y-axis
            vk::FrontFace::eCounterClockwise, VK_FALSE, f32(0), f32(0), f32(0), f32(1) };

    // Multisampling
    auto multisampling_info =
        vk::PipelineMultisampleStateCreateInfo{ vk::PipelineMultisampleStateCreateFlags(),
            vk::SampleCountFlagBits::e1, VK_FALSE, f32(1), nullptr, VK_FALSE, VK_FALSE

        };

    // Depth and stencil testing.
    auto depth_stencil_info =
        vk::PipelineDepthStencilStateCreateInfo{ vk::PipelineDepthStencilStateCreateFlags(),
            VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE };

    auto color_blend_attachment = vk::PipelineColorBlendAttachmentState{
        VK_FALSE,
    };
    color_blend_attachment.setColorWriteMask(vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA);

    auto color_blend_info =
        vk::PipelineColorBlendStateCreateInfo{ vk::PipelineColorBlendStateCreateFlags(), VK_FALSE,
            vk::LogicOp::eCopy, 1, &color_blend_attachment, { 0.f, 0.f, 0.f, 0.f } };
    // Dynamic state
    vk::DynamicState dynamic_states[]{ vk::DynamicState::eViewport, vk::DynamicState::eLineWidth };

    auto dynamic_state_info =
        vk::PipelineDynamicStateCreateInfo{ vk::PipelineDynamicStateCreateFlags(), 2,
            dynamic_states };

    auto vertex_input_info =
        vk::PipelineVertexInputStateCreateInfo{ vk::PipelineVertexInputStateCreateFlags(), 0, 0 };

    // Input assembly
    auto input_assembly =
        vk::PipelineInputAssemblyStateCreateInfo{ vk::PipelineInputAssemblyStateCreateFlags(),
            vk::PrimitiveTopology::eTriangleList, VK_FALSE };

    // Pipeline layout
    layout_ = device->get().createPipelineLayoutUnique({
        vk::PipelineLayoutCreateFlags(),
        0,
        nullptr,
        0,
        nullptr,
    });

    // Create pipeline
    vk_pipeline_ = device->get().createGraphicsPipelineUnique(nullptr,
        { vk::PipelineCreateFlags(), u32(shader_stages_.size()), shader_stages_.data(),
            &vertex_input_info, &input_assembly, nullptr, &viewport_info, &rasterizer_info,
            &multisampling_info, &depth_stencil_info, &color_blend_info, nullptr, layout_.get(),
            render_pass, 0, nullptr, -1 });

    debug::log("Graphics pipeline created!");
}

void pipeline::process(vk::CommandBuffer buff)
{

    if (ready()) {
        debug::log("Pipeline process");
        bind_buffer(buff);
        on_process(buff);
    }
}

void pipeline::bind_buffer(vk::CommandBuffer buff)
{
    buff.bindPipeline(vk::PipelineBindPoint::eGraphics, vk_pipeline_.get());
}

} // namespace ban
