#pragma once

#include <banner/gfx/swapchain.hpp>
#include <banner/gfx/device.hpp>
#include <banner/gfx/vk_utils.hpp>

namespace ban {
inline vk::SurfaceFormatKHR
choose_swap_format(const std::vector<vk::SurfaceFormatKHR>& formats)
{
    for (const auto& f : formats) {
        if (f.format == vk::Format::eB8G8R8A8Srgb
            && f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return f;
        }
    }
    return formats[0];
}

inline vk::PresentModeKHR
choose_swap_mode(const std::vector<vk::PresentModeKHR>& modes)
{
    for (const auto& mode : modes) {
        if (mode == vk::PresentModeKHR::eMailbox) {
            return mode;
        }
    }
    return vk::PresentModeKHR::eFifoRelaxed;
}

vk::Extent2D
choose_swap_extent(const vk::SurfaceCapabilitiesKHR& capabilities, vk::Extent2D extent)
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        extent.width = (std::max)(capabilities.minImageExtent.width,
            (std::min)(capabilities.maxImageExtent.width, extent.width));
        extent.height = (std::max)(capabilities.minImageExtent.height,
            (std::min)(capabilities.maxImageExtent.height, extent.height));
        return extent;
    }
}

swapchain::swapchain(device* dev, vk::SurfaceKHR surface, vk::Extent2D extent)
{
    surface_ = surface;
    device_ = dev;
    extent_ = extent;

    create_vk_swapchain();
}

swapchain::~swapchain() {}

void
swapchain::create_vk_swapchain()
{
    auto [capabilities, formats, modes]
        = vk_utils::get_surface_info(device_->get_gpu(), surface_);

    mode_ = choose_swap_mode(modes);
    format_ = choose_swap_format(formats);
    extent_ = choose_swap_extent(capabilities, extent_);

    auto image_count = std::min<u32>(
        std::max<u32>(capabilities.minImageCount, 2), capabilities.maxImageCount);

    std::vector<u32> used_indices{ device_->get_queues().graphics_index };
    auto sharing_mode = vk::SharingMode::eExclusive;

    if (!device_->get_queues().is_same()) {
        used_indices = { device_->get_queues().graphics_index,
            device_->get_queues().present_index };
        sharing_mode = vk::SharingMode::eConcurrent;
    }

    auto old_swapchain = vk_swapchain_.get();

    vk::SwapchainCreateInfoKHR swapchain_info{ {}, surface_, image_count, format_.format,
        format_.colorSpace, extent_, 1, vk::ImageUsageFlagBits::eColorAttachment,
        sharing_mode, u32(used_indices.size()), used_indices.data(),
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        vk::CompositeAlphaFlagBitsKHR::eOpaque, mode_, VK_TRUE, old_swapchain };

    vk_swapchain_ = device_->get().createSwapchainKHRUnique(swapchain_info);

    create_imageviews();

    if (old_swapchain) {
        device_->get().destroySwapchainKHR(old_swapchain);
    }
}

void
swapchain::create_imageviews()
{
    const auto images = device_->get().getSwapchainImagesKHR(vk_swapchain_.get());

    imageviews_.clear();

    for (const auto& img : images) {
        imageviews_.push_back(device_->get().createImageViewUnique({
            vk::ImageViewCreateFlags(), img, vk::ImageViewType::e2D, format_.format,
            vk::ComponentMapping(), // R,G,B,A: Identity Components
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor,
                0, // baseMiplevel
                1, // level count
                0, // baseArraylayers
                1) // layers count
        }));
    }
}

void
swapchain::resize(vk::Extent2D extent)
{
    device_->get_queues().wait();
    extent_ = extent;
    create_vk_swapchain();
    on_recreate.fire();
}
} // namespace ban