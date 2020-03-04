
#include <banner/gfx/device.hpp>
#include <banner/gfx/pipeline.hpp>
#include <banner/gfx/render_pass.hpp>
#include <banner/gfx/swapchain.hpp>
#include <banner/util/debug.hpp>

namespace ban {

pipeline::pipeline()
    : info_{}
{
    info_.vertex_input_state = { {}, 0, nullptr, 0, nullptr };
    info_.input_assembly = { {}, vk::PrimitiveTopology::eTriangleList, false };
    info_.rasterizer = { {}, false, false, vk::PolygonMode::eFill, {},
        vk::FrontFace::eCounterClockwise };
    info_.rasterizer.setLineWidth(1.f);
    info_.multisample = { {}, vk::SampleCountFlagBits::e1, false, 1.0 };
}

void pipeline::create(render_pass* render_pass)
{

    debug::log("Trying to create pipeline!");

    const auto device = render_pass->get_swap()->get_device();

    const auto [viewport, rasterization, multisample, depth_stencil, input_assembly,
        vertex_input_state, color_blend, dynamic_state] = info_;

    layout_ = device->vk().createPipelineLayoutUnique({});

    vk_pipeline_ = device->vk().createGraphicsPipelineUnique({},
        { {}, u32(shader_stages_.size()), shader_stages_.data(), &vertex_input_state,
            &input_assembly, nullptr, &viewport, &rasterization, &multisample,
            nullptr /*&depth_stencil*/, &color_blend, nullptr, layout_.get(), render_pass->vk(),
            0 });
}

vk::PipelineColorBlendAttachmentState pipeline::default_color_blend_attachment()
{
    return { VK_TRUE, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha,
        vk::BlendOp::eAdd, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA };
}

void pipeline::add_color_blend_attachment(vk::PipelineColorBlendAttachmentState state)
{
    color_blend_attachments_.push_back(state);
    info_.color_blend.setAttachmentCount(color_blend_attachments_.size());
    info_.color_blend.setPAttachments(color_blend_attachments_.data());
}


void pipeline::process(vk::CommandBuffer buffer, vk::Extent2D extent)
{
    if (ready()) {
        set_viewport(buffer, extent);
        bind_buffer(buffer);
        on_process(buffer);
    }
}

void pipeline::set_viewport(vk::CommandBuffer buffer, vk::Extent2D extent)
{
    // viewport
    viewport_ = { 0.f, 0.f, f32(extent.width), f32(extent.height), 0.f, 1.f };

    // scissor
    scissor_ = { { 0, 0 }, extent };

    buffer.setViewport(0, viewport_);
    buffer.setScissor(0, scissor_);
}

void pipeline::bind_buffer(vk::CommandBuffer buffer)
{
    buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vk_pipeline_.get());
}

} // namespace ban
