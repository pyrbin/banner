#pragma once

#include <banner/core/types.hpp>
#include <banner/util/signal.hpp>
#include <vulkan/vulkan.hpp>

namespace ban {

struct subpass;
struct swapchain;
struct render_pass;

struct pipeline
{
    friend struct subpass;

    using uptr = std::unique_ptr<pipeline>;
    using list = std::vector<uptr>;
    using shader_stages = std::vector<vk::PipelineShaderStageCreateInfo>;

    explicit pipeline();

    struct create_info
    {
        vk::PipelineViewportStateCreateInfo viewport = {};
        vk::PipelineRasterizationStateCreateInfo rasterizer = {};
        vk::PipelineMultisampleStateCreateInfo multisample = {};
        vk::PipelineDepthStencilStateCreateInfo depth_stencil = {};
        vk::PipelineInputAssemblyStateCreateInfo input_assembly = {};
        vk::PipelineVertexInputStateCreateInfo vertex_input_state = {};
        vk::PipelineColorBlendStateCreateInfo color_blend = {};
        vk::PipelineDynamicStateCreateInfo dynamic_state = {};
    };

    bool ready() const { return on_process != nullptr && vk_pipeline_; }

    static vk::PipelineColorBlendAttachmentState default_color_blend_attachment();
    void add_color_blend_attachment(
        vk::PipelineColorBlendAttachmentState state = default_color_blend_attachment());

    void add_shader(const std::string& name, vk::ShaderStageFlagBits flag, vk::ShaderModule module)
    {
        shader_stages_.emplace_back(
            vk::PipelineShaderStageCreateFlags(), flag, module, name.c_str());
    }
    void add_vertex(const std::string& name, vk::ShaderModule module)
    {
        add_shader(name, vk::ShaderStageFlagBits::eVertex, module);
    }
    void add_fragment(const std::string& name, vk::ShaderModule module)
    {
        add_shader(name, vk::ShaderStageFlagBits::eFragment, module);
    }

    fn<void(vk::CommandBuffer)> on_process;

private:
    void create(render_pass*);
    void process(vk::CommandBuffer buffer, vk::Extent2D extent);
    void bind_buffer(vk::CommandBuffer buffer);
    void set_viewport(vk::CommandBuffer buffer, vk::Extent2D extent);

    vk::UniquePipeline vk_pipeline_;
    vk::UniquePipelineLayout layout_;

    create_info info_;
    std::vector<vk::VertexInputBindingDescription> vertex_input_bindings_ = {};
    std::vector<vk::VertexInputAttributeDescription> vertex_input_attributes_ = {};
    std::vector<vk::PipelineColorBlendAttachmentState> color_blend_attachments_ = {};
    std::vector<vk::DynamicState> dynamic_states_ = {};

    shader_stages shader_stages_{};

    bool flip_y_{ true };

    vk::Viewport viewport_;
    vk::Rect2D scissor_;
};


} // namespace ban
