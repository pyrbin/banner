#include <banner/gfx/graphics.hpp>
#include <banner/gfx/res/buffer.hpp>
#include <banner/gfx/vk_utils.hpp>

namespace bnr {
using vk_utils::success;

std::tuple<vk::Buffer, VmaAllocation> buffer::create_transfer(
    graphics* ctx, const void* data, u32 size, vk::BufferUsageFlagBits usage)
{
    vk::Buffer buffer;
    VmaAllocation allocation;
    const auto allocator = ctx->memory()->allocator();

    VmaAllocationCreateInfo allocation_create_info{
        .flags{ VMA_ALLOCATION_CREATE_MAPPED_BIT },
        .usage{ VMA_MEMORY_USAGE_CPU_ONLY },
    };

    vk::BufferCreateInfo buffer_create_info{ {}, size,
        usage | vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive };

    if (!success(vmaCreateBuffer(allocator,
            reinterpret_cast<VkBufferCreateInfo*>(&buffer_create_info),
            &allocation_create_info, reinterpret_cast<VkBuffer*>(&buffer), &allocation,
            nullptr))) {
        debug::fatal("Failed to create staging buffer!");
    }

    void* mapped{};

    if (!success(vmaMapMemory(allocator, allocation, &mapped))) {
        debug::fatal("Failed to map staging buffer!");
    }

    memcpy(mapped, data, u32(size));
    vmaUnmapMemory(allocator, allocation);

    return std::make_tuple(buffer, allocation);
}

buffer::buffer(graphics* ctx, const void* data, u32 size, vk::BufferUsageFlagBits usage)
    : ctx_{ ctx }
{
    auto allocator = ctx_->memory()->allocator();

    vk::BufferCreateInfo create_info{ {}, size,
        usage | vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive };

    VmaAllocationCreateInfo allocation_create_info{
        .usage{ VMA_MEMORY_USAGE_GPU_ONLY },
    };

    if (!success(vmaCreateBuffer(allocator,
            reinterpret_cast<VkBufferCreateInfo*>(&create_info), &allocation_create_info,
            reinterpret_cast<VkBuffer*>(&vk_buffer_), &allocation_, &allocation_info_))) {
        debug::fatal("Failed to create buffer!");
    }

    if (data) {
        auto [staging_buffer, staging_allocation] =
            buffer::create_transfer(ctx_, data, size, usage);

        ctx_->command([&](vk::CommandBuffer buff) {
            vk::BufferCopy region{ 0u, 0u, size };
            buff.copyBuffer(staging_buffer, vk_buffer_, region);
        });

        vmaDestroyBuffer(allocator, staging_buffer, staging_allocation);
    }

    descriptor_.buffer = vk_buffer_;
    descriptor_.offset = 0;
    descriptor_.range = size;
}

buffer::~buffer()
{
    vmaDestroyBuffer(ctx()->memory()->allocator(), vk_buffer_, allocation_);
}

void buffer::draw(vk::CommandBuffer buffer)
{
    buffer.bindVertexBuffers(0, vk(), vk::DeviceSize(0));
    buffer.draw(3, 1, 0, 0);
}
} // namespace bnr