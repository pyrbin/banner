#pragma once

#include <vector>

#include <banner/core/types.hpp>
#include <banner/util/signal.hpp>
#include <vulkan/vulkan.hpp>

namespace ban {
struct device;

struct swapchain
{
    using ptr = swapchain*;

    explicit swapchain(device::ptr device, vk::SurfaceKHR surface, vk::Extent2D extent);
    ~swapchain();

    struct swapchain_data
    {
        std::vector<vk::Image> images;
        std::vector<vk::UniqueImageView> views;
    };

    vk::ResultValue<u32> aquire_image(vk::Semaphore sem, vk::Fence fen, u32 timeout);
    void resize(vk::Extent2D extent);
    signal<void()> on_recreate;

    auto get() const { return vk_swapchain_.get(); }
    auto get_device() { return device_; }

    const auto& get_format() const { return format_; }
    const auto& get_extent() const { return extent_; }

    auto get_image_count() const { return u32(data_.images.size()); }
    const auto& get_data() const { return data_; }


private:
    device::ptr device_;
    vk::SurfaceKHR surface_;

    vk::SurfaceFormatKHR format_;
    vk::Extent2D extent_;
    vk::PresentModeKHR mode_;

    vk::UniqueSwapchainKHR vk_swapchain_;

    swapchain_data data_;

    void create_vk_swapchain();
    void create_imageviews();
};


} // namespace ban
