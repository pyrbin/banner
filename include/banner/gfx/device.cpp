#pragma once
#include <unordered_set>

#include <banner/defs.hpp>
#include <banner/gfx/device.hpp>
#include <banner/gfx/vk_utils.hpp>

namespace ban {
device::device(const vk::PhysicalDevice& gpu, const vk::SurfaceKHR surface, const options opts)

{
    std::vector<vk::DeviceQueueCreateInfo> queue_infos;

    const auto indices = vk_utils::get_queue_family_info(gpu, surface);
    const std::unordered_set<u32> unique_family_info{ indices.graphics_family.value(),
        indices.present_family.value() };

    f32 priority = 1.0f;

    for (auto idx : unique_family_info) {
        queue_infos.push_back({ {}, idx, 1, &priority });
    }

    gpu_ = gpu;
    features_ = gpu_.getFeatures();
    props_ = gpu_.getMemoryProperties();

    vk::DeviceCreateInfo device_info{ vk::DeviceCreateFlags(), u32(queue_infos.size()),
        queue_infos.data(), u32(opts.layers.size()), opts.layers.data(),
        u32(opts.extensions.size()), opts.extensions.data(), &features_ };

    vk_device_ = gpu_.createDeviceUnique(device_info);

    queues_ = std::make_unique<queues>(
        this, indices.graphics_family.value(), indices.present_family.value());
}

device::~device() {}
} // namespace ban