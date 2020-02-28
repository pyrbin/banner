#pragma once

#include <banner/gfx/renderer.hpp>
#include <banner/gfx/swapchain.hpp>
#include <banner/gfx/device.hpp>

namespace ban {
renderer::renderer(swapchain* swapc)
    : swapchain_{ swapc }
{
    const device_ptr device = swapchain_->get_device();

    // Create fences
    for (u32 i = 0; i < swapchain_->get_image_count(); i++) {
        vk::FenceCreateInfo info = { vk::FenceCreateFlagBits::eSignaled };
        fences_.push_back(device->get().createFence(info));
    }

    // Create semaphore
    sync_.aquire_sema = device->get().createSemaphoreUnique(vk::SemaphoreCreateInfo{});
    sync_.render_sema = device->get().createSemaphoreUnique(vk::SemaphoreCreateInfo{});
}

void
renderer::pre_render()
{
    index_ = swapchain_->aquire_image(sync_.aquire_sema.get(), nullptr, UINT64_MAX).value;
    swapchain_->get_device()->get().waitForFences(fences_[index_], true, UINT64_MAX);
    swapchain_->get_device()->get().resetFences(fences_[index_]);
}
void
renderer::render()
{
    const device_ptr device = swapchain_->get_device();

    vk::SubmitInfo submit_info;
    submit_info.setWaitSemaphoreCount(1);
    submit_info.setPWaitSemaphores(&sync_.aquire_sema.get());
    vk::PipelineStageFlags flags[]
        = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submit_info.setPWaitDstStageMask(flags);
    submit_info.setCommandBufferCount(1);
    submit_info.setPCommandBuffers(&cmd_buffers_[index_]);
    submit_info.setSignalSemaphoreCount(1);
    submit_info.setPSignalSemaphores(&sync_.render_sema.get());

    device->get_queues().submit(submit_info, fences_[index_]);

    vk::PresentInfoKHR present_info;
    present_info.setPImageIndices(&index_);
    present_info.setSwapchainCount(1);
    present_info.setPSwapchains(&swapchain_->get());
    present_info.setWaitSemaphoreCount(1);
    present_info.setPWaitSemaphores(&sync_.render_sema.get());
    present_info.setPResults(nullptr);

    VULKAN_CHECK(device->get_queues().present(present_info));
}

void
renderer::wait() const
{
    swapchain_->get_device()->get().waitForFences(fences_, true, -1);
}
} // namespace ban
