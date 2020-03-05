#include <algorithm>

#include <banner/defs.hpp>
#include <banner/gfx/device.hpp>
#include <banner/gfx/pipeline.hpp>
#include <banner/gfx/render_pass.hpp>
#include <banner/gfx/swapchain.hpp>

namespace bnr {
subpass::ref subpass::add_pipeline(pipeline* pipeline)
{
    pipelines_.emplace_back(pipeline);
    if (activated()) {
        pipeline->create(owner_);
    }
    return *this;
}

void subpass::create()
{
    activated_ = true;
    for (auto& pipeline : pipelines_) {
        pipeline->create(owner_);
    }
}

render_pass::render_pass(swapchain* swapchain)
    : swapchain_{ swapchain }
{
    set_clear_color({ 0.13f, 0.03f, 0.11f, 1 });
}

void render_pass::create()
{
    create_render_pass();
    create_framebuffers();

    swapchain_->on_recreate.connect<&render_pass::create_framebuffers>(*this);

    for (auto& subpass : subpasses_) {
        subpass->create();
    }
}

render_pass::~render_pass()
{
    for (auto& sp : subpasses_) {
        sp.reset();
    }
    subpasses_.clear();
}


void render_pass::process(u32 frame, vk::CommandBuffer buffer)
{

    if (!vk_render_pass_ || subpasses_.size() <= 0)
        return;

    vk::RenderPassBeginInfo begin_info{ vk(), framebuffers_[frame].get(),
        vk::Rect2D(vk::Offset2D(0, 0), extent_), u32(1), &clear_value_ };

    buffer.beginRenderPass(&begin_info, vk::SubpassContents::eInline);

    for (auto& subpass : subpasses_) {
        subpass->process(buffer, extent_);
    }

    buffer.endRenderPass();
}

void render_pass::add(subpass* subpass)
{
    subpasses_.emplace_back(std::move(subpass));
    subpass->set_render_pass(this);
}

void render_pass::add(pipeline* pipeline, u32 subpass_idx)
{
    subpasses_.at(subpass_idx)->add_pipeline(pipeline);
}

void render_pass::add(attachment attachment)
{
    attachment.description_.setFormat(swapchain_->get_format().format);
    attachments_.push_back(attachment.vk());
}

void render_pass::add(subpass::dependency dependency)
{
    dependencies_.push_back(dependency.vk());
}

void render_pass::create_render_pass()
{
    const auto device = &swapchain_->get_device()->vk();

    vector<vk::SubpassDescription> subpasses;
    std::transform(subpasses_.begin(), subpasses_.end(), std::back_inserter(subpasses),
        [&](uptr<subpass>& pass) { return pass->vk(); });

    if (vk_render_pass_) {
        vk_render_pass_.release();
    }

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
        vector<vk::ImageView> framebuffer_attachments = { img_view.get() };

        framebuffers_.push_back(device->createFramebufferUnique(
            { {}, vk(), u32(framebuffer_attachments.size()),
                framebuffer_attachments.data(), extent_.width, extent_.height, 1 }));
    }
}
} // namespace bnr
