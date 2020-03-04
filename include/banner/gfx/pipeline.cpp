
#include <banner/gfx/device.hpp>
#include <banner/gfx/pipeline.hpp>
#include <banner/gfx/render_pass.hpp>
#include <banner/gfx/swapchain.hpp>
#include <banner/util/debug.hpp>

namespace ban {

pipeline::pipeline() {}

void pipeline::create(render_pass* render_pass)
{
    const auto device = render_pass->get_swap()->get_device();
    const auto extent = render_pass->get_swap()->get_extent();
}

void pipeline::process(vk::CommandBuffer buffer, vk::Extent2D extent)
{
    if (ready()) {
        set_viewport(extent);
        bind_buffer(buffer);
        on_process(buffer);
    }
}

void pipeline::set_viewport(vk::Extent2D extent)
{
    // viewport
    viewport_ = vk::Viewport{ f32(0), f32(extent.height), f32(extent.width), -f32(extent.width),
        f32(0), f32(1) };

    // scissor
    scissor_ = vk::Rect2D{ { 0, 0 }, { extent.width, extent.height } };

    // viewport state
    auto viewport_info =
        vk::PipelineViewportStateCreateInfo{ vk::PipelineViewportStateCreateFlags(), 1, &viewport_,
            1, &scissor_ };
}

void pipeline::bind_buffer(vk::CommandBuffer buffer)
{
    buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vk_pipeline_.get());
}

} // namespace ban
