#pragma once

#include <vulkan/vulkan.hpp>
#include <banner/core/types.hpp>
#include <banner/gfx/vk_utils.hpp>

namespace ban {

struct swapchain;

struct renderer
{
    using cmd_buffers = vk::CommandBuffer*;
    using fences = std::vector<vk::Fence>;

    renderer(swapchain* swapc);

    struct synchronization
    {
        vk::UniqueSemaphore aquire_sema{ nullptr };
        vk::UniqueSemaphore render_sema{ nullptr };
    };

    swapchain* get_swapchain() { return swapchain_; }

    void pre_render();
    void render();

    void set_cmd_buffers(cmd_buffers buffers) { cmd_buffers_ = buffers; }

    void wait() const;

    u32 get_index() const { return index_; }

private:
    swapchain* swapchain_;
    fences fences_;
    synchronization sync_;
    cmd_buffers cmd_buffers_;
    u32 index_{ 0 };
};
} // namespace ban
