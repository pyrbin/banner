#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <banner/core/types.hpp>
#include <banner/util/signal.hpp>

namespace ban {
struct device;

struct swapchain
{
    explicit swapchain(device* dev, vk::SurfaceKHR surface, vk::Extent2D extent);
    ~swapchain();

    vk::SwapchainKHR get() const { return vk_swapchain_.get(); }
    const std::vector<vk::UniqueImageView>& get_imageviews() const { return imageviews_; }
    const vk::SurfaceFormatKHR& get_format() const { return format_; }
    const vk::Extent2D& get_extent() const { return extent_; }

    void resize(vk::Extent2D extent);
    signal<void()> on_recreate;

private:
    device* device_;
    vk::SurfaceKHR surface_;

    vk::SurfaceFormatKHR format_;
    vk::Extent2D extent_;
    vk::PresentModeKHR mode_;

    vk::UniqueSwapchainKHR vk_swapchain_;
    std::vector<vk::UniqueImageView> imageviews_;

    void create_vk_swapchain();
    void create_imageviews();
};
} // namespace ban