#pragma once

#include <string>
#include <vector>

#include <banner/gfx/device.hpp>

namespace ban {
struct subpass;

struct pipeline
{
    friend class subpass;

    using ptr = pipeline*;
    using list = std::vector<pipeline*>;
    using shader_stages = std::vector<vk::PipelineShaderStageCreateInfo>;

    explicit pipeline(device::ptr device);
    ~pipeline() {}

    void build(vk::Extent2D extent, vk::RenderPass render_pass);

    bool ready() const { return on_process != nullptr && vk_pipeline_; }

    void add_shader_stage(
        const std::string& name, vk::ShaderStageFlagBits flags, vk::ShaderModule module)
    {
        shader_stages_.emplace_back(
            vk::PipelineShaderStageCreateFlags(), flags, module, name.c_str());
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
    void process(vk::CommandBuffer buff)
    {
        if (ready()) {
            bind_buffer(buff);
            on_process(buff);
        }
    }

    void bind_buffer(vk::CommandBuffer buff)
    {
        buff.bindPipeline(vk::PipelineBindPoint::eGraphics, vk_pipeline_.get());
    }

    device::ptr device_;
    vk::UniquePipeline vk_pipeline_;
    shader_stages shader_stages_{};

    vk::Extent2D extent_;
    vk::UniquePipelineLayout layout_;
    vk::Viewport viewport_;
    vk::Rect2D scissor_;
};
} // namespace ban