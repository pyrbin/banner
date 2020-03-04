#pragma once
#include <banner/core/types.hpp>
#include <banner/util/signal.hpp>
#include <vulkan/vulkan.hpp>

namespace ban {

struct swapchain;
struct renderer;

struct subpass
{
    friend class render_pass;

    using uptr = std::unique_ptr<subpass>;
    using list = std::vector<uptr>;

    explicit subpass(vk::SubpassDescription description)
        : description_{ description }
    {}

    auto& get_desc() const { return description_; }

    void process(vk::CommandBuffer buffer) { on_process.fire(buffer); }
    signal<void(vk::CommandBuffer)> on_process;


private:
    vk::SubpassDescription description_;
};

struct render_pass
{
    friend class renderer;

    using framebuffers = std::vector<vk::UniqueFramebuffer>;
    using dependencies = std::vector<vk::SubpassDependency>;
    using attachments = std::vector<vk::AttachmentDescription>;

    explicit render_pass();

    auto vk() { return vk_render_pass_.get(); }

    void add(subpass* subpass) { subpasses_.emplace_back(std::move(subpass)); }
    void add(vk::AttachmentDescription description)
    {
        attachments_.push_back(std::move(description));
    }

    void create(swapchain* swapchain);
    void process(u32 frame, vk::CommandBuffer buffer);

private:
    void create_render_pass();
    void create_framebuffers();

    swapchain* swapchain_;
    vk::UniqueRenderPass vk_render_pass_;

    vk::Extent2D extent_;
    vk::ClearValue clear_value_;

    framebuffers framebuffers_;
    attachments attachments_;
    dependencies dependencies_;
    subpass::list subpasses_;
};

} // namespace ban
