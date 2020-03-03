#include <banner/core/types.hpp>
#include <banner/gfx/pipeline.hpp>

namespace ban {
pipeline::pipeline(device::ptr device)
    : device_{ device }
{}

void pipeline::build(vk::Extent2D extent, vk::RenderPass render_pass)
{
    vk::PipelineVertexInputStateCreateInfo vertex_info{ {}, 0, nullptr, 0, nullptr };
    vk::PipelineInputAssemblyStateCreateInfo input_assembly{ {},
        vk::PrimitiveTopology::eTriangleList, VK_FALSE };


    vk::Viewport viewport{ 0, 0, f32(extent.width), f32(extent.height), 0, 1 };

    vk::Rect2D scissor{ { 0, 0 }, { extent.width, extent.height } };

    vk::PipelineViewportStateCreateInfo viewport_info{ {}, 1, &viewport, 1, &scissor };

    vk::PipelineMultisampleStateCreateInfo multisample{ {}, vk::SampleCountFlagBits::e1, VK_FALSE,
        0, nullptr, VK_FALSE, VK_FALSE };

    vk::PipelineRasterizationStateCreateInfo rasterization{ {}, VK_FALSE, VK_FALSE,
        vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
        VK_FALSE, 0, 0, 0, 1 };

    vk::PipelineDepthStencilStateCreateInfo depth_stencil{ {}, VK_FALSE, VK_FALSE,
        vk::CompareOp::eNever, VK_FALSE, VK_FALSE, {}, {}, 0, 0 };
}
} // namespace ban
