#pragma once

#include <vector>

#include <banner/core/types.hpp>
#include <vulkan/vulkan.hpp>

namespace ban {
struct device
{

    struct queues;

    struct options
    {
        std::vector<cstr> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        std::vector<cstr> layers{ "VK_LAYER_KHRONOS_validation" };
    };

    explicit device(
        const vk::PhysicalDevice& device, const vk::SurfaceKHR surface, const options opts);

    auto vk() const { return vk_device_.get(); }
    auto get_gpu() const { return gpu_; }

    const auto& get_queues() const { return *queues_.get(); }
    const auto& get_features() { return features_; }
    const auto& get_props() { return props_; }

    struct queues
    {
        inline queues(const device* device, u32 gidx, u32 pidx)
            : graphics_queue{ device->vk().getQueue(gidx, 0) }
            , present_queue{ device->vk().getQueue(pidx, 0) }
            , graphics_index{ gidx }
            , present_index{ pidx }
        {}

        const vk::Queue graphics_queue;
        const vk::Queue present_queue;

        const u32 graphics_index;
        const u32 present_index;

        [[nodiscard]] void submit(vk::SubmitInfo info, vk::Fence fence) const
        {
            return graphics_queue.submit(info, fence);
        }

        [[nodiscard]] vk::Result present(vk::PresentInfoKHR info) const
        {
            return present_queue.presentKHR(info);
        }

        [[nodiscard]] void wait() const noexcept
        {
            graphics_queue.waitIdle();
            present_queue.waitIdle();
        }

        [[nodiscard]] bool is_same() const { return graphics_index == present_index; }
    };

private:
    vk::PhysicalDevice gpu_;
    vk::PhysicalDeviceFeatures features_;
    vk::PhysicalDeviceMemoryProperties props_;
    vk::UniqueDevice vk_device_;
    uptr<queues> queues_;
};
} // namespace ban
