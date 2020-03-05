#pragma once

#include <algorithm>

#include <banner/core/types.hpp>
#include <banner/gfx/device.hpp>
#include <banner/gfx/graphics.hpp>
#include <banner/gfx/swapchain.hpp>
#include <vulkan/vulkan.hpp>

namespace ban {

using cmd_buffers = std::vector<vk::CommandBuffer>;
using fences = std::vector<vk::Fence>;

struct renderer
{
    using uptr = uptr<renderer>;

    struct task
    {
        using fn = fn<void(vk::CommandBuffer)>;
        using list = vector<task*>;

        task::fn process;
        cmd_buffers cmd_buffers;

        task(device*, task::fn, vk::CommandPool, u32);
        void free(device*, vk::CommandPool);
    };

    struct sync
    {
        vk::UniqueSemaphore aquire{ nullptr };
        vk::UniqueSemaphore render{ nullptr };
    };

    explicit renderer(graphics* graphics);
    ~renderer();

    void add_task(task::fn task);

    auto get_device() const { return device_; }
    auto get_swap() const { return swapchain_; }

    auto get_current() const { return current_; };
    auto get_current_buffers()
    {
        std::vector<vk::CommandBuffer> v{};
        std::transform(tasks_.begin(), tasks_.end(), std::back_inserter(v),
            [i = current_](task* t) { return t->cmd_buffers[i]; });
        return v;
    };

    auto& get_sync() const { return sync_; }
    auto& get_pool() { return cmd_pool; }

    void update();
    auto wait() const;
    auto wait(u32 idx) const;
    auto fence_reset(u32 idx) const;

private:
    bool aquire_next_image();
    void process_tasks();
    void end_frame();

    graphics* graphics_{ nullptr };
    swapchain* swapchain_{ nullptr };
    device* device_{ nullptr };

    task::list tasks_;
    vk::CommandPool cmd_pool;

    fences flight_fences_;
    sync sync_;

    u32 current_{ 0 };
};
} // namespace ban
