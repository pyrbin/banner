#include <banner/gfx/renderer.hpp>
#include <banner/gfx/vk_utils.hpp>

namespace ban {
renderer::task::task(device* device, task::fn fn, vk::CommandPool pool, u32 count)
    : process{ fn }
{
    // Allocate command buffers
    cmd_buffers.resize(count);
    cmd_buffers = device->vk().allocateCommandBuffersUnique(
        { pool, vk::CommandBufferLevel::ePrimary, count });
}

renderer::renderer(graphics* graphics)
    : graphics_{ graphics }
    , device_{ graphics->get_device() }
    , swapchain_{ graphics->get_swap() }

{
    // Create command pools
    for (u32 i = 0; i < swapchain_->get_image_count(); i++) {
        cmd_pools_.push_back(device_->vk().createCommandPoolUnique(
            { vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                device_->get_queues().present_index }));
    }

    // Create sync objects
    for (u32 i = 0; i < swapchain_->get_image_count(); i++) {
        vk::FenceCreateInfo info = { vk::FenceCreateFlagBits::eSignaled };
        fences_.push_back(device_->vk().createFence(info));
    }

    sync_.aquire = device_->vk().createSemaphoreUnique({});
    sync_.render = device_->vk().createSemaphoreUnique({});
}

renderer::~renderer()
{
    for (u32 i = 0; i < fences_.size(); i++) {
        device_->vk().destroyFence(fences_[i]);
    }
    fences_.clear();
}

void renderer::add_task(task::fn task)
{
    // Create task
    tasks_.emplace_back(device_, task, cmd_pools_.back().get(), swapchain_->get_image_count());
}

auto renderer::wait() const
{
    return swapchain_->get_device()->vk().waitForFences(fences_, true, -1);
}

auto renderer::wait(u32 idx) const
{
    return swapchain_->get_device()->vk().waitForFences(fences_[idx], true, -1);
}

auto renderer::fence_reset(u32 idx) const
{
    return swapchain_->get_device()->vk().resetFences(fences_[idx]);
}

void renderer::update()
{
    if (!aquire_next_image())
        return;

    process_tasks();
    end_frame();
}

bool renderer::aquire_next_image()
{
    if (!vk_utils::success(wait(current_)))
        return false;

    auto aquire_result = swapchain_->aquire_image(sync_.aquire.get());

    if (aquire_result.result == vk::Result::eErrorOutOfDateKHR) {
        graphics_->reload_swapchain();
        return false;
    } else if (!vk_utils::success(aquire_result)) {
        return false;
    }

    current_ = aquire_result.value;

    fence_reset(current_);

    return true;
}

void renderer::process_tasks()
{
    device_->vk().resetCommandPool(cmd_pools_[current_].get(), (vk::CommandPoolResetFlagBits)0);

    for (auto& [proc, buffs] : tasks_) {
        auto cmd_buff = buffs[current_].get();

        cmd_buff.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        if (proc) {
            proc(cmd_buff);
        }

        cmd_buff.end();
    }
}

void renderer::end_frame()
{
    auto buffers = get_current_buffers();

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

    device_->get_queues().submit(submit_info, fences_[current_]);

    // Present stage
    vk::PresentInfoKHR present_info;
    present_info.setPImageIndices(&current_);
    present_info.setSwapchainCount(1);
    present_info.setPSwapchains(&swapchain_->vk());
    present_info.setWaitSemaphoreCount(1);
    present_info.setPWaitSemaphores(&sync_.render.get());
    present_info.setPResults(nullptr);

    VULKAN_CHECK(device_->get_queues().present(present_info));
}
} // namespace ban
