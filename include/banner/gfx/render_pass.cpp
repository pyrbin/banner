#pragma once

#include <algorithm>

#include <banner/gfx/render_pass.hpp>
#include <banner/util/debug.hpp>

namespace ban {

void render_pass::add(const vk::AttachmentDescription& attachment)
{
    attachments_.push_back(attachment);
}

// void render_pass::add(const vk::SubpassDescription& description)
//{
//    subpasses_.push_back(description);
//}

void render_pass::add(const vk::SubpassDependency& dependecy)
{
    dependencies_.push_back(dependecy);
}

void render_pass::create(swapchain::ptr swap)
{
    auto device = swap->get_device();

    std::vector<vk::SubpassDescription> desc{};

    std::transform(subpasses_.begin(), subpasses_.end(), std::back_inserter(desc),
        [&](subpass* pass) { return pass->get_desc(); });

    vk_render_pass_ = device->get().createRenderPassUnique(
        { vk::RenderPassCreateFlags(), u32(attachments_.size()), attachments_.data(),
            u32(desc.size()), desc.data(), u32(dependencies_.size()), dependencies_.data() });

    extent_ = swap->get_extent();

    framebuffers_.clear();
    framebuffers_.resize(0);
    for (auto& img_view : swap->get_data().views) {
        std::vector<vk::ImageView> framebuffer_attachments = { img_view.get() };
        framebuffers_.push_back(
            device->get().createFramebufferUnique({ vk::FramebufferCreateFlags(),
                vk_render_pass_.get(), u32(framebuffer_attachments.size()),
                framebuffer_attachments.data(), extent_.width, extent_.height, 1 }));
    }

    clear_value_ =
        vk::ClearValue(vk::ClearColorValue{ std::array<float, 4>({ 1.0f, 0.0f, 1.0f, 1.0f }) });
}

void render_pass::process(u32 frame, vk::CommandBuffer buff)
{
    vk::RenderPassBeginInfo begin_info{ vk_render_pass_.get(), framebuffers_[frame].get(),
        vk::Rect2D(vk::Offset2D(0, 0), extent_), u32(2), &clear_value_ };

    buff.beginRenderPass(&begin_info, vk::SubpassContents::eInline);

    for (auto subpass : subpasses_) {
        subpass->process(buff);
    }

    buff.endRenderPass();
}
} // namespace ban
