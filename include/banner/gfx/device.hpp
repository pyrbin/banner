#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <banner/core/types.hpp>

namespace ban {
struct device
{
    struct options
    {
        std::vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        std::vector<const char*> layers{ "VK_LAYER_KHRONOS_validation" };
    };

    explicit device(const vk::PhysicalDevice& device, const vk::SurfaceKHR surface,
        const options opts);
    ~device();

    struct queues
    {
        inline queues(const device* device, u32 gidx, u32 pidx)
            : graphics_queue{ device->get().getQueue(gidx, 0) }
            , present_queue{ device->get().getQueue(pidx, 0) }
            , graphics_index{ gidx }
            , present_index{ pidx }
        {}

        const vk::Queue graphics_queue;
        const vk::Queue present_queue;

        const u32 graphics_index;
        const u32 present_index;

        [[nodiscard]] void wait() const noexcept
        {
            graphics_queue.waitIdle();
            present_queue.waitIdle();
        }

        [[nodiscard]] bool is_same() const { return graphics_index == present_index; }
    };

    vk::Device get() const { return vk_device_.get(); }
    const queues& get_queues() const { return *queues_.get(); }

    vk::PhysicalDevice get_gpu() const { return gpu_; }
    const vk::PhysicalDeviceFeatures& get_features() { return features_; }

private:
    vk::PhysicalDevice gpu_;
    vk::PhysicalDeviceFeatures features_;

    vk::UniqueDevice vk_device_;
    std::unique_ptr<queues> queues_;
};

using device_ptr = device*;
} // namespace ban