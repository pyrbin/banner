#include <algorithm>

#include <banner/defs.hpp>
#include <banner/gfx/device.hpp>
#include <banner/gfx/pipeline.hpp>
#include <banner/gfx/render_pass.hpp>
#include <banner/gfx/swapchain.hpp>

namespace ban {

void subpass::add_pipeline(pipeline* pipeline)
{
    pipelines_.emplace_back(pipeline);
    on_process.connect<&pipeline::process>(pipeline);
    on_create.connect<&pipeline::create>(pipeline);
}


render_pass::render_pass(swapchain* swapchain)
    : swapchain_{ swapchain }
{
    set_clear_color({ 0, 1, 0, 1 });
}

void render_pass::create()
{
    create_render_pass();
    create_framebuffers();
}

void render_pass::process(u32 frame, vk::CommandBuffer buffer)
{
    if (!swapchain_)
        return;

    vk::RenderPassBeginInfo begin_info{ vk(), framebuffers_[frame].get(),
        vk::Rect2D(vk::Offset2D(0, 0), extent_), u32(1), &clear_value_ };

    buffer.beginRenderPass(&begin_info, vk::SubpassContents::eInline);

    for (auto& subpass : subpasses_) {
        subpass->process(buffer, extent_);
    }

    buffer.endRenderPass();
}

void render_pass::add(attachment attachment)
{
    attachment.description_.setFormat(swapchain_->get_format().format);
    attachments_.push_back(attachment.vk());
}

void render_pass::add(dependency dependency)
{
    dependencies_.push_back(dependency.vk());
}

void render_pass::create_render_pass()
{
    const auto device = &swapchain_->get_device()->vk();

    std::vector<vk::SubpassDescription> subpasses;
    std::transform(subpasses_.begin(), subpasses_.end(), std::back_inserter(subpasses),
        [&](subpass::uptr& pass) { return pass->vk(); });

    vk_render_pass_ = device->createRenderPassUnique(
        { {}, u32(attachments_.size()), attachments_.data(), u32(subpasses.size()),
            subpasses.data(), u32(dependencies_.size()), dependencies_.data() });

    ASSERT(vk_render_pass_, "Failed to create render pass!");
}
void render_pass::create_framebuffers()
{
    const auto device = &swapchain_->get_device()->vk();

    extent_ = swapchain_->get_extent();

    framebuffers_.clear();
    framebuffers_.resize(0);
    for (auto& img_view : swapchain_->get_data().views) {
        std::vector<vk::ImageView> framebuffer_attachments = { img_view.get() };
        framebuffers_.push_back(
            device->createFramebufferUnique({ {}, vk(), u32(framebuffer_attachments.size()),
                framebuffer_attachments.data(), extent_.width, extent_.height, 1 }));
    }
}


} // namespace ban
