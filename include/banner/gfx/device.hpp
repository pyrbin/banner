#pragma once

#include <banner/core/types.hpp>
#include <vulkan/vulkan.hpp>

namespace bnr {
struct device
{

    struct options
    {
        vector<cstr> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        vector<cstr> layers{ "VK_LAYER_KHRONOS_validation" };
    };

    explicit device(const vk::PhysicalDevice& device, const vk::SurfaceKHR surface,
        const options opts);

    auto vk() const { return vk_device_.get(); }
    auto physical() const { return vk_physical_; }

    struct queue_data
    {
        inline queue_data(const device* device, u32 gidx, u32 pidx)
            : graphics_queue{ device->vk().getQueue(gidx, 0) }
            , present_queue{ device->vk().getQueue(pidx, 0) }
            , graphics_index{ gidx }
            , present_index{ pidx }
        {}

        auto& graphics() { return graphics_queue; }
        auto& present() { return present_queue; }

        const u32 graphics_index;
        const u32 present_index;

        void submit(vk::SubmitInfo info, vk::Fence fence) const
        {
            return graphics_queue.submit(info, fence);
        }

        vk::Result present(vk::PresentInfoKHR info) const
        {
            return present_queue.presentKHR(info);
        }

        void wait() const noexcept
        {
            graphics_queue.waitIdle();
            present_queue.waitIdle();
        }

        bool is_same() const { return graphics_index == present_index; }

    private:
        const vk::Queue graphics_queue;
        const vk::Queue present_queue;
    };

    const auto& queue() const { return *queue_.get(); }
    const auto& features() { return features_; }
    const auto& props() { return props_; }

private:
    vk::PhysicalDevice vk_physical_;
    vk::PhysicalDeviceFeatures features_;
    vk::PhysicalDeviceMemoryProperties props_;
    vk::UniqueDevice vk_device_;
    uptr<queue_data> queue_;
};
} // namespace bnr
