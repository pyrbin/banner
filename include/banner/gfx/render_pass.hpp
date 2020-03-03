#pragma once

#include <vector>

#include <banner/core/types.hpp>
#include <banner/gfx/device.hpp>
#include <banner/gfx/pipeline.hpp>
#include <banner/gfx/swapchain.hpp>

namespace ban {
struct subpass
{
    friend class render_pass;

    explicit subpass();
    ~subpass();

    void add_pipeline(pipeline::ptr pipeline)
    {
        pipelines_.push_back(pipeline);
        on_process.connect<&pipeline::process>(pipeline);
    }

    auto& get_desc() const { return description_; }

    void process(vk::CommandBuffer buff) { on_process.fire(buff); }

    signal<void(vk::CommandBuffer)> on_process;

private:
    vk::SubpassDescription description_;
    pipeline::list pipelines_;
};

struct render_pass
{
    explicit render_pass(device::ptr device);
    ~render_pass();

    void add(const vk::AttachmentDescription& attachment);
    // void add(const vk::SubpassDescription& description);
    void add(const vk::SubpassDependency& dependecy);
    void add_subpass(subpass* subpass) { subpasses_.push_back(subpass); }

    void create(swapchain::ptr);
    void process(u32 frame, vk::CommandBuffer buff);

    auto get() { return vk_render_pass_.get(); }

private:
    device::ptr device_;
    vk::UniqueRenderPass vk_render_pass_;

    vk::Extent2D extent_;
    vk::ClearValue clear_value_;

    std::vector<vk::UniqueFramebuffer> framebuffers_;
    std::vector<vk::AttachmentDescription> attachments_;
    std::vector<vk::SubpassDependency> dependencies_;
    std::vector<subpass*> subpasses_;
};
} // namespace ban