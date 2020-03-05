#pragma once
#include <banner/core/types.hpp>
#include <banner/gfx/pipeline.hpp>
#include <banner/util/signal.hpp>
#include <vulkan/vulkan.hpp>

namespace bnr {
struct swapchain;
struct render_pass;
struct renderer;

struct subpass
{
    friend class render_pass;

    using list = vector<uptr<subpass>>;
    using ref = subpass&;

    explicit subpass()
        : description_{}
    {
        description_.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    }

    auto vk() const { return description_; }
    auto activated() const { return activated_; }

    struct dependency
    {
        friend class render_pass;
        using ref = dependency&;

        dependency(vk::SubpassDependency dep = {})
            : dependency_{ dep }
        {}

        dependency(u32 src, u32 dst)
            : dependency_{}
        {
            set_subpass(src, dst);
        }

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

    ref add_pipeline(pipeline* pipeline);

    ref set_color_attachment(vk::AttachmentReference attachment)
    {
        color_attachments_.push_back(attachment);
        description_.setColorAttachmentCount(u32(color_attachments_.size()));
        description_.setPColorAttachments(color_attachments_.data());
        return *this;
    }
    ref set_resolve_attachment(vk::AttachmentReference attachment)
    {
        resolve_attachment_ = attachment;
        description_.setPResolveAttachments(&resolve_attachment_);
        return *this;
    }
    ref set_depth_stencil_attachment(vk::AttachmentReference attachment)
    {
        depth_stencil_attachment_ = attachment;
        description_.setPDepthStencilAttachment(&depth_stencil_attachment_);
        return *this;
    }
    ref set_preserve_attachment(u32 attachment)
    {
        preserve_attachments_.push_back(attachment);
        description_.setPreserveAttachmentCount(u32(preserve_attachments_.size()));
        description_.setPPreserveAttachments(preserve_attachments_.data());
        return *this;
    }

    ref set_render_pass(render_pass* render_pass)
    {
        owner_ = render_pass;
        return *this;
    }

    void create();

    void process(vk::CommandBuffer buffer, vk::Extent2D extent)
    {
        for (auto& pipeline : pipelines_) {
            pipeline->process(buffer, extent);
        }
    }

private:
    render_pass* owner_{ nullptr };
    bool activated_{ false };

    vector<vk::AttachmentReference> color_attachments_;
    vk::AttachmentReference resolve_attachment_;
    vk::AttachmentReference depth_stencil_attachment_;
    vector<u32> preserve_attachments_;

    vk::SubpassDescription description_;
    pipeline::list pipelines_;
};

struct render_pass
{
    friend class renderer;

    using framebuffers = vector<vk::UniqueFramebuffer>;
    using dependencies = vector<vk::SubpassDependency>;
    using attachments = vector<vk::AttachmentDescription>;

    struct attachment
    {
        friend class render_pass;
        using ref = attachment&;

        attachment(vk::AttachmentDescription description = {})
            : description_{ description }
        {}

        attachment(vk::Format format)
            : description_{}
        {
            description_.setFormat(format);
        }

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

        ref set_format(vk::Format format)
        {
            description_.setFormat(format);
            return *this;
        }

    private:
        vk::AttachmentDescription description_;
    };

    explicit render_pass(swapchain* swapchain);
    ~render_pass();

    auto vk() const { return vk_render_pass_.get(); }
    auto get_clear_color() const { return clear_value_; }
    auto get_swap() const { return swapchain_; }
    auto get_subpass(u32 idx) const { return subpasses_.at(idx).get(); }

    void add(subpass* subpass);
    void add(attachment attachment);
    void add(subpass::dependency dependency);
    void add(pipeline* pipeline, u32 subpass_idx = 0);

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
} // namespace bnr
