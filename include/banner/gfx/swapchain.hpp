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

    struct swapchain_data
    {
        std::vector<vk::Image> images;
        std::vector<vk::UniqueImageView> views;
    };

    vk::SwapchainKHR get() const { return vk_swapchain_.get(); }
    const vk::SurfaceFormatKHR& get_format() const { return format_; }
    const vk::Extent2D& get_extent() const { return extent_; }

    u32 get_image_count() const { return u32(data_.images.size()); }
    const swapchain_data& get_data() const { return data_; }

    device* get_device() { return device_; }

    vk::ResultValue<u32> aquire_image(vk::Semaphore sem, vk::Fence fen, u32 timeout);

    void resize(vk::Extent2D extent);

    signal<void()> on_recreate;

private:
    device* device_;
    vk::SurfaceKHR surface_;

    vk::SurfaceFormatKHR format_;
    vk::Extent2D extent_;
    vk::PresentModeKHR mode_;

    vk::UniqueSwapchainKHR vk_swapchain_;

    swapchain_data data_;

    void create_vk_swapchain();
    void create_imageviews();
};

using swapchain_ptr = swapchain*;


} // namespace ban
