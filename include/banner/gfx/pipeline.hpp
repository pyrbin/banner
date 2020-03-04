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

    bool ready() const { return on_process != nullptr && vk_pipeline_; }

    fn<void(vk::CommandBuffer)> on_process;

private:
    void create(render_pass*);
    void process(vk::CommandBuffer buffer, vk::Extent2D extent);
    void bind_buffer(vk::CommandBuffer buffer);
    void set_viewport(vk::Extent2D extent);

    vk::UniquePipeline vk_pipeline_;
    shader_stages shader_stages_{};

    bool flip_y{ true };

    vk::Viewport viewport_;
    vk::Rect2D scissor_;
};


} // namespace ban
