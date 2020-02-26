#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "common/types.hpp"

namespace tde {

class vulkan_graphics;
class renderer;

struct tri_renderer
{
    tri_renderer(vulkan_graphics* graphics, renderer* renderer);

    vk::UniqueCommandPool _cmd_pool;
    std::vector<vk::CommandBuffer> _cmd_buffers;
    vk::UniqueRenderPass _render_pass;
    std::vector<vk::Framebuffer> _framebuffers;
    vk::UniquePipelineLayout _pipeline_layout;
    vk::UniquePipeline _pipeline;

    vulkan_graphics* _graphics;
    renderer* _renderer;

    void update(f32);

    void createRenderPass();
    void createFramebuffers();
    void createPipelineLayout();
    void createPipeline();
};

} // namespace tde
