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

    create_cmd_buffers();
}

void
renderer::create_cmd_buffers()
{
    const device_ptr device = swapchain_->get_device();

    vk::CommandPoolCreateInfo create_info;
    create_info.setQueueFamilyIndex(device->get_queues().present_index);
    cmd_pool_ = device->get().createCommandPoolUnique(create_info);

    vk::CommandBufferAllocateInfo alloc_info;
    alloc_info.setCommandPool(cmd_pool_.get());
    alloc_info.setLevel(vk::CommandBufferLevel::ePrimary);
    alloc_info.setCommandBufferCount(swapchain_->get_image_count());

    cmd_buffers_ = device->get().allocateCommandBuffers(alloc_info);

    // Prepare data for recording command buffers
    vk::CommandBufferBeginInfo begin_info;
    begin_info.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

    // Note: contains value for each subresource range
    vk::ClearColorValue clear_values = { std::array<float, 4>{ 1.f, 0.f, 0.f, 1.f } };

    vk::ImageSubresourceRange subresource_range;
    subresource_range.setAspectMask(vk::ImageAspectFlagBits::eColor);
    subresource_range.setBaseMipLevel(0);
    subresource_range.setLevelCount(1);
    subresource_range.setBaseArrayLayer(0);
    subresource_range.setLayerCount(1);

    auto idx = swapchain_->get_device()->get_queues().present_index;

    for (u32 i{ 0 }; i < swapchain_->get_image_count(); i++) {
        // -------------------------------------------------------------------------;
        vk::ImageMemoryBarrier present_to_clearbarrier;
        present_to_clearbarrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        present_to_clearbarrier.setDstAccessMask(vk::AccessFlagBits::eMemoryRead);
        present_to_clearbarrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
        present_to_clearbarrier.setNewLayout(vk::ImageLayout::ePresentSrcKHR);
        present_to_clearbarrier.setSrcQueueFamilyIndex(idx);
        present_to_clearbarrier.setDstQueueFamilyIndex(idx);
        present_to_clearbarrier.setImage(swapchain_->get_data().images[i]);
        present_to_clearbarrier.setSubresourceRange(subresource_range);
        // -------------------------------------------------------------------------;
        cmd_buffers_[i].begin(begin_info);
        cmd_buffers_[i].pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer, (vk::DependencyFlagBits)0, {}, {},
            present_to_clearbarrier);
        cmd_buffers_[i].clearColorImage(swapchain_->get_data().images[i],
            vk::ImageLayout::eTransferDstOptimal, &clear_values, 1, &subresource_range);
        cmd_buffers_[i].pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eBottomOfPipe, (vk::DependencyFlagBits)0, {}, {},
            present_to_clearbarrier);
        cmd_buffers_[i].end();
    }
}

void
renderer::pre_render()
{
    // index_ = swapchain_->aquire_image(sync_.aquire_sema.get(), nullptr,
    // UINT64_MAX).value; swapchain_->get_device()->get().waitForFences(fences_[index_],
    // true, UINT64_MAX); swapchain_->get_device()->get().resetFences(fences_[index_]);
}

void
renderer::render()
{
    index_ = swapchain_->aquire_image(sync_.aquire_sema.get(), nullptr, UINT64_MAX).value;
    const device_ptr device = swapchain_->get_device();

    vk::SubmitInfo submit_info;
    submit_info.setWaitSemaphoreCount(1);
    submit_info.setPWaitSemaphores(&sync_.aquire_sema.get());
    vk::PipelineStageFlags flags[] = { vk::PipelineStageFlagBits::eTransfer };
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
