#pragma once

#include <algorithm>

#include <banner/gfx/device.hpp>
#include <banner/gfx/swapchain.hpp>
#include <vulkan/vulkan.hpp>

namespace ban {
using cmd_pools = std::vector<vk::UniqueCommandPool>;
using cmd_buffers = std::vector<vk::UniqueCommandBuffer>;

using fences = std::vector<vk::Fence>;

struct renderer
{
    using ptr = renderer*;

    struct task
    {
        using fn = fn<void(vk::CommandBuffer)>;
        using list = std::vector<task>;

        task::fn process;
        cmd_buffers cmd_buffers;

        task(device::ptr, task::fn, vk::CommandPool, u32);
    };

    struct sync
    {
        vk::UniqueSemaphore aquire{ nullptr };
        vk::UniqueSemaphore render{ nullptr };
    };

    explicit renderer(swapchain::ptr swapchain);
    ~renderer();

    void add_task(task::fn task);

    auto get_device() const { return device_; }
    auto get_current() const { return current_; };
    auto get_current_buffers()
    {
        std::vector<vk::CommandBuffer> v{};
        std::transform(tasks_.begin(), tasks_.end(), std::back_inserter(v),
            [i = current_](task& t) { return t.cmd_buffers[i].get(); });
        return v;
    };

    auto get_swap() const { return swapchain_; }

    auto& get_sync() const { return sync_; }
    auto& get_pool(u32 idx) { return cmd_pools_.at(idx); }

    void render();
    auto wait() const;
    auto wait(u32 idx) const;

private:
    bool aquire();
    void process();
    void present();

    device::ptr device_{ nullptr };
    swapchain::ptr swapchain_{ nullptr };

    task::list tasks_;
    cmd_pools cmd_pools_;

    fences fences_;
    sync sync_;

    u32 current_{ 0 };
};
} // namespace ban