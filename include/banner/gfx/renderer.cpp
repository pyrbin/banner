#include <banner/gfx/renderer.hpp>
#include <banner/gfx/vk_utils.hpp>

namespace bnr {
renderer::task::task(bnr::device* device, task::fn fn, vk::CommandPool pool, u32 count)
    : process{ fn }
{
    // Allocate command buffers
    cmd_buffers.resize(count);
    cmd_buffers = device->vk().allocateCommandBuffers(
        { pool, vk::CommandBufferLevel::ePrimary, count });
}

void renderer::task::free(bnr::device* device, vk::CommandPool pool)
{
    device->vk().freeCommandBuffers(pool, cmd_buffers);
}

renderer::renderer(graphics* ctx)
    : ctx_{ ctx }

{
    // Create command pool
    cmd_pool = (device()->vk().createCommandPool(
        { vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            device()->queue().present_index }));

    // Create sync objects
    for (u32 i = 0; i < swapchain()->image_count(); i++) {
        vk::FenceCreateInfo info = { vk::FenceCreateFlagBits::eSignaled };
        flight_fences_.push_back(device()->vk().createFence(info));
    }

    sync_.aquire = device()->vk().createSemaphoreUnique({});
    sync_.render = device()->vk().createSemaphoreUnique({});
}

auto renderer::wait() const
{
    return device()->vk().waitForFences(flight_fences_, true, -1);
}

auto renderer::wait(u32 idx) const
{
    return device()->vk().waitForFences(flight_fences_[idx], true, -1);
}

auto renderer::reset_fence(u32 idx) const
{
    return device()->vk().resetFences(flight_fences_[idx]);
}

renderer::~renderer()
{
    wait();

    for (auto& task : tasks_) {
        task->free(device(), cmd_pool);
        delete task;
    }

    device()->vk().destroyCommandPool(cmd_pool);

    for (u32 i{ 0 }; i < flight_fences_.size(); i++) {
        device()->vk().destroyFence(flight_fences_[i]);
    }

    flight_fences_.clear();
    tasks_.clear();
}

void renderer::add_task(task::fn task)
{
    // Create task
    tasks_.push_back(
        new renderer::task(device(), task, cmd_pool, swapchain()->image_count()));
}

void renderer::render()
{
    if (tasks_.size() <= 0)
        return;

    if (!aquire_next_image())
        return;

    process_tasks();
    end_frame();
}

bool renderer::aquire_next_image()
{
    if (!vk_utils::success(wait(current_)))
        return false;

    auto aquire_result = swapchain()->aquire_image(sync_.aquire.get());

    if (aquire_result.result == vk::Result::eErrorOutOfDateKHR) {
        ctx()->reload_swapchain();
        return false;
    } else if (!vk_utils::success(aquire_result)) {
        return false;
    }

    current_ = aquire_result.value;

    reset_fence(current_index());

    return true;
}

void renderer::process_tasks()
{
    device()->vk().resetCommandPool(cmd_pool, (vk::CommandPoolResetFlagBits)0);

    for (auto& task : tasks_) {
        auto& [proc, buffs] = *task;

        auto cmd_buff = buffs[current_];

        cmd_buff.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        if (proc) {
            proc(cmd_buff);
        }

        cmd_buff.end();
    }
}

void renderer::end_frame()
{
    auto buffers = current_buffers();

    // Submit stage
    vk::SubmitInfo submit_info;
    submit_info.setWaitSemaphoreCount(1);
    submit_info.setPWaitSemaphores(&sync_.aquire.get());
    vk::PipelineStageFlags flags[] = { vk::PipelineStageFlagBits::eTransfer };
    submit_info.setPWaitDstStageMask(flags);
    submit_info.setCommandBufferCount(u32(buffers.size()));
    submit_info.setPCommandBuffers(buffers.data());
    submit_info.setSignalSemaphoreCount(1);
    submit_info.setPSignalSemaphores(&sync_.render.get());

    device()->queue().submit(submit_info, flight_fences_[current_]);

    // Present stage
    vk::PresentInfoKHR present_info;
    present_info.setPImageIndices(&current_);
    present_info.setSwapchainCount(1);
    present_info.setPSwapchains(&swapchain()->vk());
    present_info.setWaitSemaphoreCount(1);
    present_info.setPWaitSemaphores(&sync_.render.get());
    present_info.setPResults(nullptr);

    VULKAN_CHECK(device()->queue().present(present_info));
}
} // namespace bnr
