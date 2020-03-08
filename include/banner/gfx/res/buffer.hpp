#pragma once

#include <tuple>

#include <banner/core/types.hpp>
#include <banner/gfx/memory.hpp>
#include <banner/gfx/res/resource.hpp>
#include <banner/gfx/res/vertex.hpp>
#include <vulkan/vulkan.hpp>

namespace bnr {
struct graphics;

struct buffer : resource
{
    using list = vector<buffer>;

    explicit buffer(graphics* ctx, const void* data, u32 size,
        vk::BufferUsageFlagBits usage = vk::BufferUsageFlagBits::eVertexBuffer);

    ~buffer();

    static std::tuple<vk::Buffer, VmaAllocation> create_transfer(graphics* ctx,
        const void* data, u32 size,
        vk::BufferUsageFlagBits usage = vk::BufferUsageFlagBits::eVertexBuffer);

    auto vk() const { return vk_buffer_; }
    auto ctx() { return ctx_; }

    auto valid() const { return (bool)vk_buffer_; }
    auto size() const { return descriptor_.range; }

    void draw(vk::CommandBuffer buf);

private:
    graphics* ctx_;

    vk::Buffer vk_buffer_;
    vk::DescriptorBufferInfo descriptor_;

    VmaAllocation allocation_;
    VmaAllocationInfo allocation_info_;
};
} // namespace bnr