#include <banner/gfx/device.hpp>
#include <banner/gfx/graphics.hpp>
#include <banner/gfx/pipeline.hpp>
#include <banner/gfx/render_pass.hpp>
#include <banner/gfx/swapchain.hpp>
#include <banner/util/debug.hpp>
#include <banner\defs.hpp>

namespace bnr {
pipeline::pipeline()
    : info_{}
{
    info_.vertex_input_state = { {}, 0, nullptr, 0, nullptr };
    info_.input_assembly = { {}, vk::PrimitiveTopology::eTriangleList, false };
    info_.rasterizer = { {}, false, false, vk::PolygonMode::eFill, {},
        vk::FrontFace::eCounterClockwise };
    info_.rasterizer.setLineWidth(1.f);
    info_.multisample = { {}, vk::SampleCountFlagBits::e1, false, 1.0 };

    info_.viewport.setScissorCount(1);
    info_.viewport.setViewportCount(1);
    info_.viewport.setPScissors(&scissor_);
    info_.viewport.setPViewports(&viewport_);
}

void pipeline::create(bnr::subpass* subpass_ptr)
{
    set_subpass(subpass_ptr);

    const auto device = subpass()->render_pass()->ctx()->device();
    const auto extent = subpass()->render_pass()->extent();

    set_viewport(nullptr, { extent.width, extent.height });

    const auto [viewport, rasterization, multisample, depth_stencil, input_assembly,
        vertex_input_state, color_blend, dynamic_state] = info_;

    vk_layout_ = device->vk().createPipelineLayoutUnique({});

    vk_pipeline_ = device->vk().createGraphicsPipelineUnique({},
        { {}, u32(shader_stages_.size()), shader_stages_.data(), &vertex_input_state,
            &input_assembly, nullptr, &viewport, &rasterization, &multisample,
            nullptr /*&depth_stencil*/, &color_blend, nullptr, vk_layout_.get(),
            subpass()->render_pass()->vk(), 0 });

    ASSERT(vk_pipeline_, "Failed to create pipeline!");

    debug::log("Created a pipeline!");
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


void pipeline::set_vertex_input_bindings(
    const vector<vk::VertexInputBindingDescription>& bindings)
{
    vertex_input_bindings_ = { bindings };
    info_.vertex_input_state.setVertexBindingDescriptionCount(
        vertex_input_bindings_.size());
    info_.vertex_input_state.setPVertexBindingDescriptions(vertex_input_bindings_.data());
}


void pipeline::set_vertex_input_attributes(
    const vector<vk::VertexInputAttributeDescription>& descriptions)
{
    vertex_input_attributes_ = { descriptions };
    info_.vertex_input_state.setVertexAttributeDescriptionCount(
        vertex_input_attributes_.size());
    info_.vertex_input_state.setPVertexAttributeDescriptions(
        vertex_input_attributes_.data());
}

void pipeline::process(vk::CommandBuffer buffer, uv2 size)
{
    if (ready()) {
        set_viewport(buffer, size);
        bind_buffer(buffer);
        on_process(buffer);
    }
}

void pipeline::set_viewport(vk::CommandBuffer buffer, uv2 size)
{
    // viewport
    viewport_ = { 0.f, 0.f, f32(size.x), f32(size.y), 0.f, 1.f };

    // scissor
    scissor_ = { { 0, 0 }, { size.x, size.y } };

    if (buffer) {
        buffer.setViewport(0, viewport_);
        buffer.setScissor(0, scissor_);
    }
}

void pipeline::bind_buffer(vk::CommandBuffer buffer)
{
    buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vk_pipeline_.get());
}
} // namespace bnr
