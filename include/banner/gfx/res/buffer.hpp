#pragma once

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

    explicit buffer(
        graphics* ctx, vk::BufferUsageFlagBits usage, const void* data, u32 size);

    ~buffer();

    auto vk() const { return vk_buffer_; }
    auto ctx() { return ctx_; }

    auto valid() const { return (bool)vk_buffer_; }
    auto size() const { return descriptor_.range; }

    void draw(vk::CommandBuffer buf, const vector<vertex>& vertices);

private:
    graphics* ctx_;

    vk::Buffer vk_buffer_;
    vk::DescriptorBufferInfo descriptor_;

    VmaAllocation allocation_;
    VmaAllocationInfo allocation_info_;
};

} // namespace bnr
