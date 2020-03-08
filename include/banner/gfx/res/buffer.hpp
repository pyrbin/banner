#pragma once

#include <tuple>

#include <banner/core/types.hpp>
#include <banner/gfx/memory.hpp>
#include <banner/gfx/res/resource.hpp>
#include <vulkan/vulkan.hpp>

namespace bnr {
struct graphics;

struct buffer : resource
{
    using list = vector<buffer>;

    explicit buffer(graphics* ctx, const void* data, u32 size,
        vk::BufferUsageFlagBits usage = vk::BufferUsageFlagBits::eVertexBuffer,
        bool gpu_only = true);

    ~buffer();

    auto vk() const { return vk_buffer_; }
    auto ctx() { return ctx_; }

    auto valid() const { return (bool)vk_buffer_; }
    auto size() const { return descriptor_.range; }

private:
    static std::tuple<vk::Buffer, VmaAllocation> buffer::allocate_vk_buffer(graphics* ctx,
        const void* data, u32 size, vk::BufferUsageFlags usage, bool mapped,
        VmaMemoryUsage memory_usage);

    graphics* ctx_;

    vk::Buffer vk_buffer_;
    vk::DescriptorBufferInfo descriptor_;

    VmaAllocation allocation_;
    VmaAllocationInfo allocation_info_;
};
} // namespace bnr