#pragma once

#include <banner/core/types.hpp>
#include <banner/util/signal.hpp>
#include <vulkan/vulkan.hpp>

namespace bnr {
struct subpass;
struct swapchain;
struct render_pass;

struct pipeline
{
    friend struct subpass;

    using uptr = uptr<pipeline>;
    using list = vector<uptr>;
    using shader_stages = vector<vk::PipelineShaderStageCreateInfo>;
    using cb_signature = void(vk::CommandBuffer);

    explicit pipeline();

    struct create_info
    {
        vk::PipelineViewportStateCreateInfo viewport{};
        vk::PipelineRasterizationStateCreateInfo rasterizer{};
        vk::PipelineMultisampleStateCreateInfo multisample{};
        vk::PipelineDepthStencilStateCreateInfo depth_stencil{};
        vk::PipelineInputAssemblyStateCreateInfo input_assembly{};
        vk::PipelineVertexInputStateCreateInfo vertex_input_state{};
        vk::PipelineColorBlendStateCreateInfo color_blend{};
        vk::PipelineDynamicStateCreateInfo dynamic_state{};
    };

    bool ready() const { return !on_process_sig.is_empty() && vk_pipeline_; }

    static vk::PipelineColorBlendAttachmentState default_color_blend_attachment();
    void add_color_blend_attachment(
        vk::PipelineColorBlendAttachmentState state = default_color_blend_attachment());

    void add_shader(cstr name, vk::ShaderStageFlagBits flag, vk::ShaderModule module)
    {
        shader_stages_.push_back({ {}, flag, std::move(module), name });
    }
    void add_vertex(cstr name, vk::ShaderModule& module)
    {
        add_shader(name, vk::ShaderStageFlagBits::eVertex, module);
    }
    void add_fragment(cstr name, vk::ShaderModule& module)
    {
        add_shader(name, vk::ShaderStageFlagBits::eFragment, module);
    }

    template<typename F>
    void on_process(F&& cb)
    {
        on_process_sig.connect(std::move(cb));
    }

private:
    void create(render_pass*);
    void process(vk::CommandBuffer buffer, vk::Extent2D extent);
    void bind_buffer(vk::CommandBuffer buffer);
    void set_viewport(vk::CommandBuffer buffer, vk::Extent2D extent);

    signal<cb_signature> on_process_sig;

    vk::UniquePipeline vk_pipeline_;
    vk::UniquePipelineLayout layout_;

    create_info info_;
    vector<vk::VertexInputBindingDescription> vertex_input_bindings_ = {};
    vector<vk::VertexInputAttributeDescription> vertex_input_attributes_ = {};
    vector<vk::PipelineColorBlendAttachmentState> color_blend_attachments_ = {};
    vector<vk::DynamicState> dynamic_states_ = {};

    shader_stages shader_stages_{};

    bool flip_y_{ true };

    vk::Viewport viewport_;
    vk::Rect2D scissor_;
};
} // namespace bnr
