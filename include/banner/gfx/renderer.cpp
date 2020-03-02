#include <banner/gfx/renderer.hpp>
#include <banner/gfx/vk_utils.hpp>

namespace ban {
renderer::task::task(device::ptr device, task::fn fn, vk::CommandPool pool, u32 count)
    : process{ fn }
{
    // Allocate command buffers
    cmd_buffers.resize(count);
    cmd_buffers = device->get().allocateCommandBuffersUnique(
        { pool, vk::CommandBufferLevel::ePrimary, count });
}

renderer::renderer(swapchain::ptr swapchain)
    : swapchain_{ swapchain }
    , device_{ swapchain->get_device() }

{
    // Create command pools
    for (u32 i = 0; i < swapchain_->get_image_count(); i++) {
        cmd_pools_.push_back(device_->get().createCommandPoolUnique(
            { vk::CommandPoolCreateFlags(), device_->get_queues().present_index }));
    }

    // Create sync objects
    for (u32 i = 0; i < swapchain_->get_image_count(); i++) {
        vk::FenceCreateInfo info = { vk::FenceCreateFlagBits::eSignaled };
        fences_.push_back(device_->get().createFence(info));
    }

    sync_.aquire = device_->get().createSemaphoreUnique({});
    sync_.render = device_->get().createSemaphoreUnique({});
}

renderer::~renderer()
{
    for (u32 i = 0; i < fences_.size(); i++) {
        device_->get().destroyFence(fences_[i]);
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
    return swapchain_->get_device()->get().waitForFences(fences_, true, -1);
}

auto renderer::wait(u32 idx) const
{
    return swapchain_->get_device()->get().waitForFences(fences_[idx], true, -1);
}

bool renderer::aquire()
{
    if (!vk_utils::success(wait(current_)))
        return false;

    auto image_index = swapchain_->aquire_image(sync_.aquire.get(), nullptr, -1);

    if (!vk_utils::success(image_index)) {
        return false;
    }

    current_ = image_index.value;
    swapchain_->get_device()->get().resetFences(fences_[current_]);
    return true;
}

void renderer::process()
{
    device_->get().resetCommandPool(cmd_pools_[current_].get(), (vk::CommandPoolResetFlagBits)0);

    for (auto& [proc, buffs] : tasks_) {
        auto cmd_buff = buffs[current_].get();

        cmd_buff.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

        if (proc) {
            proc(cmd_buff);
        }

        cmd_buff.end();
    }
}

void renderer::present()
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
    present_info.setPSwapchains(&swapchain_->get());
    present_info.setWaitSemaphoreCount(1);
    present_info.setPWaitSemaphores(&sync_.render.get());
    present_info.setPResults(nullptr);

    VULKAN_CHECK(device_->get_queues().present(present_info));
}

void renderer::render()
{
    if (aquire()) {
        process();
        present();
    }
}
} // namespace ban