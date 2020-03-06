#pragma once

#include <algorithm>

#include <banner/core/types.hpp>
#include <banner/gfx/device.hpp>
#include <banner/gfx/graphics.hpp>
#include <banner/gfx/swapchain.hpp>
#include <vulkan/vulkan.hpp>

namespace bnr {

struct renderer
{
    using cmd_buffers = vector<vk::CommandBuffer>;
    using fences = vector<vk::Fence>;

    struct task
    {
        using fn = fn<void(vk::CommandBuffer)>;
        using list = vector<task*>;

        task::fn process;
        cmd_buffers cmd_buffers;

        task(device*, task::fn, vk::CommandPool, u32);
        void free(device*, vk::CommandPool);
    };

    struct synchronization
    {
        vk::UniqueSemaphore aquire{ nullptr };
        vk::UniqueSemaphore render{ nullptr };
    };

    explicit renderer(graphics* ctx);
    ~renderer();

    void add_task(task::fn task);

    auto ctx() const { return ctx_; }
    auto device() const { return ctx_->device(); }
    auto swapchain() const { return ctx_->swapchain(); }

    u32 current_index() const { return current_; };

    auto current_buffers()
    {
        vector<vk::CommandBuffer> v{};
        std::transform(tasks_.begin(), tasks_.end(), std::back_inserter(v),
            [i = current_](task* t) { return t->cmd_buffers[i]; });
        return v;
    };

    auto& sync() const { return sync_; }
    auto& pool() { return cmd_pool; }

    void render();
    auto wait() const;
    auto wait(u32 idx) const;
    auto reset_fence(u32 idx) const;

private:
    bool aquire_next_image();
    void process_tasks();
    void end_frame();

    graphics* ctx_{ nullptr };

    task::list tasks_;
    vk::CommandPool cmd_pool;

    fences flight_fences_;
    synchronization sync_;

    u32 current_{ 0 };
};
} // namespace bnr
