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

    auto vk() { return vk_pipeline_.get(); }
    auto layout() { return vk_layout_.get(); }
    auto subpass() { return subpass_; }

    auto ready() const { return on_process && vk_pipeline_; }
    void set_subpass(bnr::subpass* subpass) { subpass_ = subpass; }

    static vk::PipelineColorBlendAttachmentState default_color_blend_attachment();

    void add_color_blend_attachment(
        vk::PipelineColorBlendAttachmentState state = default_color_blend_attachment());

    void set_vertex_input_bindings(const vector<vk::VertexInputBindingDescription>&);

    void set_vertex_input_attributes(const vector<vk::VertexInputAttributeDescription>&);

    void add_shader(cstr name, vk::ShaderStageFlagBits flag, vk::ShaderModule module)
    {
        shader_stages_.push_back({ {}, flag, std::move(module), name });
    }
    void add_vertex_shader(cstr name, vk::ShaderModule& module)
    {
        add_shader(name, vk::ShaderStageFlagBits::eVertex, module);
    }
    void add_fragment_shader(cstr name, vk::ShaderModule& module)
    {
        add_shader(name, vk::ShaderStageFlagBits::eFragment, module);
    }

    fn<cb_signature> on_process;

private:
    void create(bnr::subpass*);
    void process(vk::CommandBuffer buffer, uv2 extent);
    void bind_buffer(vk::CommandBuffer buffer);
    void set_viewport(vk::CommandBuffer buffer, uv2 extent);

    vk::UniquePipeline vk_pipeline_;
    vk::UniquePipelineLayout vk_layout_;

    bnr::subpass* subpass_;

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
