#pragma once

#include <banner/gfx/device.hpp>
#include <banner/gfx/swapchain.hpp>
#include <banner/gfx/vk_utils.hpp>

namespace ban {
inline vk::SurfaceFormatKHR choose_format(const std::vector<vk::SurfaceFormatKHR>& formats)
{
    for (const auto& f : formats) {
        if (f.format == vk::Format::eB8G8R8A8Srgb &&
            f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return f;
        }
    }
    return formats[0];
}

inline vk::PresentModeKHR choose_present_mode(const std::vector<vk::PresentModeKHR>& modes)
{
    for (const auto& mode : modes) {
        if (mode == vk::PresentModeKHR::eMailbox) {
            return mode;
        }
    }
    return vk::PresentModeKHR::eFifoRelaxed;
}

vk::Extent2D choose_extent(const vk::SurfaceCapabilitiesKHR& capabilities, vk::Extent2D extent)
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

swapchain::swapchain(device* device, vk::SurfaceKHR surface, vk::Extent2D extent)
{
    surface_ = surface;
    device_ = device;
    extent_ = extent;

    create_vk_swapchain();
}

swapchain::~swapchain() {}

void swapchain::create_vk_swapchain()
{
    auto [capabilities, formats, modes] = vk_utils::get_surface_info(device_->get_gpu(), surface_);

    mode_ = choose_present_mode(modes);
    format_ = choose_format(formats);
    extent_ = choose_extent(capabilities, extent_);

    auto image_count =
        std::min<u32>(std::max<u32>(capabilities.minImageCount, 2), capabilities.maxImageCount);

    auto old_swapchain = vk_swapchain_.get();

    // Determine transformation to use (preferring no transform)
    vk::SurfaceTransformFlagBitsKHR surface_transform;
    if (capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
        surface_transform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
    } else {
        surface_transform = capabilities.currentTransform;
    }

    vk::SwapchainCreateInfoKHR create_info;
    create_info.setSurface(surface_);
    create_info.setMinImageCount(image_count);
    create_info.setImageFormat(format_.format);
    create_info.setImageColorSpace(format_.colorSpace);
    create_info.setImageExtent(extent_);
    create_info.setImageArrayLayers(1);
    create_info.setImageUsage(
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst);
    create_info.setImageSharingMode(vk::SharingMode::eExclusive);
    create_info.setQueueFamilyIndexCount(0);
    create_info.setPQueueFamilyIndices(nullptr);
    create_info.setPreTransform(surface_transform);
    create_info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    create_info.setPresentMode(mode_);
    create_info.setClipped(VK_TRUE);
    create_info.setOldSwapchain(old_swapchain);

    vk_swapchain_ = device_->vk().createSwapchainKHRUnique(create_info);

    create_imageviews();

    if (old_swapchain) {
        device_->vk().destroySwapchainKHR(old_swapchain);
    }
}

void swapchain::create_imageviews()
{
    data_.images.clear();
    data_.views.clear();
    data_.images = device_->vk().getSwapchainImagesKHR(vk_swapchain_.get());

    for (const auto& image : data_.images) {
        data_.views.push_back(device_->vk().createImageViewUnique({
            vk::ImageViewCreateFlags(), image, vk::ImageViewType::e2D, format_.format,
            vk::ComponentMapping(), // R,G,B,A: Identity Components
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor,
                0, // baseMiplevel
                1, // level count
                0, // baseArraylayers
                1) // layers count
        }));
    }
}

void swapchain::resize(vk::Extent2D extent)
{
    device_->vk().waitIdle();

    extent_ = extent;
    create_vk_swapchain();

    on_recreate.fire();
}

vk::ResultValue<u32> swapchain::aquire_image(vk::Semaphore sem, vk::Fence fen, u32 timeout)
{
    return device_->vk().acquireNextImageKHR(vk(), timeout, sem, fen);
}
} // namespace ban
