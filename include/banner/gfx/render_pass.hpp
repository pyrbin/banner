#pragma once
#include <banner/core/types.hpp>
#include <banner/util/signal.hpp>
#include <vulkan/vulkan.hpp>

namespace ban {

struct swapchain;
struct render_pass;
struct renderer;
struct pipeline;

struct subpass
{
    friend class render_pass;

    using uptr = uptr<subpass>;
    using list = std::vector<uptr>;

    explicit subpass()
        : description_{}
    {
        description_.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    }

    auto vk() const { return description_; }

    void add_pipeline(pipeline* pipeline);

    void set_color_attachment(vk::AttachmentReference attachment)
    {
        color_attachments_.push_back(attachment);
        description_.setColorAttachmentCount(u32(color_attachments_.size()));
        description_.setPColorAttachments(color_attachments_.data());
    }
    void set_resolve_attachment(vk::AttachmentReference attachment)
    {
        resolve_attachment_ = attachment;
        description_.setPResolveAttachments(&resolve_attachment_);
    }
    void set_depth_stencil_attachment(vk::AttachmentReference attachment)
    {
        depth_stencil_attachment_ = attachment;
        description_.setPDepthStencilAttachment(&depth_stencil_attachment_);
    }
    void set_preserve_attachment(u32 attachment)
    {
        preserve_attachments_.push_back(attachment);
        description_.setPreserveAttachmentCount(u32(preserve_attachments_.size()));
        description_.setPPreserveAttachments(preserve_attachments_.data());
    }

    void set_render_pass(render_pass* render_pass) { owner = render_pass; }

    void process(vk::CommandBuffer buffer, vk::Extent2D extent)
    {
        if (activated_)
            on_process.fire(buffer, extent);
    }

    signal<void(render_pass*)> on_create;
    signal<void(vk::CommandBuffer, vk::Extent2D)> on_process;

private:
    render_pass* owner{ nullptr };
    bool activated_{ false };

    std::vector<vk::AttachmentReference> color_attachments_;
    vk::AttachmentReference resolve_attachment_;
    vk::AttachmentReference depth_stencil_attachment_;
    std::vector<u32> preserve_attachments_;

    vk::SubpassDescription description_;
    pipeline::list pipelines_;
};

struct render_pass
{
    friend class renderer;

    using framebuffers = std::vector<vk::UniqueFramebuffer>;
    using dependencies = std::vector<vk::SubpassDependency>;
    using attachments = std::vector<vk::AttachmentDescription>;

    struct attachment
    {
        friend class render_pass;
        using ref = attachment&;

        explicit attachment(vk::AttachmentDescription description = {})
            : description_{ description }
        {}

        auto vk() { return description_; }

        ref set_op(vk::AttachmentLoadOp load, vk::AttachmentStoreOp store)
        {
            description_.setLoadOp(load);
            description_.setStoreOp(store);
            return *this;
        }

        ref set_stencil_op(vk::AttachmentLoadOp load, vk::AttachmentStoreOp store)
        {
            description_.setStencilLoadOp(load);
            description_.setStencilStoreOp(store);
            return *this;
        }

        ref set_layout(vk::ImageLayout initial, vk::ImageLayout final)
        {
            description_.setInitialLayout(initial);
            description_.setFinalLayout(final);
            return *this;
        }

    private:
        vk::AttachmentDescription description_;
    };

    struct dependency
    {
        friend class render_pass;
        using ref = dependency&;

        explicit dependency(vk::SubpassDependency dep = {})
            : dependency_{ dep }
        {}

        auto vk() { return dependency_; }

        ref set_subpass(u32 src, u32 dst)
        {
            dependency_.setSrcSubpass(src);
            dependency_.setDstSubpass(dst);
            return *this;
        }

        ref set_stage_mask(vk::PipelineStageFlags src, vk::PipelineStageFlags dst)
        {
            dependency_.setSrcStageMask(src);
            dependency_.setDstStageMask(dst);
            return *this;
        }

        ref set_access_mask(vk::AccessFlags src, vk::AccessFlags dst)
        {
            dependency_.setSrcAccessMask(src);
            dependency_.setDstAccessMask(dst);
            return *this;
        }

        ref set_flags(vk::DependencyFlags flags)
        {
            dependency_.setDependencyFlags(flags);
            return *this;
        }

    private:
        vk::SubpassDependency dependency_;
    };

    explicit render_pass(swapchain* swapchain);

    auto vk() const { return vk_render_pass_.get(); }
    auto get_clear_color() const { return clear_value_; }
    auto get_swap() const { return swapchain_; }

    void add(subpass* subpass);
    void add(attachment attachment);
    void add(dependency dependency);

    void set_clear_color(std::array<f32, 4> values)
    {
        clear_value_ = vk::ClearValue(vk::ClearColorValue{ values });
    }

    void create();
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
