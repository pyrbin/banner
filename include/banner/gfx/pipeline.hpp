#pragma once

#include <string>
#include <vector>

#include <banner/gfx/device.hpp>
#include <banner/gfx/swapchain.hpp>

namespace ban {
struct subpass;

struct pipeline
{
    friend class subpass;

    using ptr = pipeline*;
    using list = std::vector<pipeline*>;
    using shader_stages = std::vector<vk::PipelineShaderStageCreateInfo>;

    void build(swapchain::ptr swap, vk::RenderPass render_pass);

    bool ready() const { return on_process != nullptr && vk_pipeline_; }

    void add_shader_stage(
        const std::string& name, vk::ShaderStageFlagBits flags, vk::ShaderModule module)
    {
        shader_stages_.push_back(
            { vk::PipelineShaderStageCreateFlags(), flags, module, name.c_str() });
    }
    void add_vertex_stage(const std::string& name, vk::ShaderModule module)
    {
        add_shader_stage(name, vk::ShaderStageFlagBits::eVertex, module);
    }
    void add_fragment_stage(const std::string& name, vk::ShaderModule module)
    {
        add_shader_stage(name, vk::ShaderStageFlagBits::eFragment, module);
    }

    fn<void(vk::CommandBuffer)> on_process;

private:
    void process(vk::CommandBuffer buff);
    void bind_buffer(vk::CommandBuffer buff);

    vk::UniquePipeline vk_pipeline_;
    shader_stages shader_stages_{};

    vk::Extent2D extent_;
    vk::UniquePipelineLayout layout_;
    vk::Viewport viewport_;
    vk::Rect2D scissor_;
};
} // namespace ban
