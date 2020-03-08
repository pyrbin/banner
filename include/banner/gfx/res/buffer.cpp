#include <banner/gfx/graphics.hpp>
#include <banner/gfx/res/buffer.hpp>
#include <banner/gfx/vk_utils.hpp>

namespace bnr {
using vk_utils::success;

buffer::buffer(graphics* ctx, const void* data, u32 size, vk::BufferUsageFlagBits usage,
    bool gpu_only)
    : ctx_{ ctx }
{
    auto allocator = ctx->memory()->allocator();
    auto mem_usage = gpu_only ? VMA_MEMORY_USAGE_GPU_ONLY : VMA_MEMORY_USAGE_CPU_ONLY;

    auto [buffer, alloc] = buffer::allocate_vk_buffer(ctx, data, size,
        (usage | vk::BufferUsageFlagBits::eTransferDst), gpu_only, mem_usage);

    vk_buffer_ = buffer;
    allocation_ = alloc;

    if (data && gpu_only) {

        auto [staging_buffer, staging_alloc] = buffer::allocate_vk_buffer(ctx, data, size,
            (usage | vk::BufferUsageFlagBits::eTransferSrc), false,
            VMA_MEMORY_USAGE_CPU_ONLY);

        ctx_->command([&](vk::CommandBuffer buff) {
            vk::BufferCopy region{ 0u, 0u, size };
            buff.copyBuffer(staging_buffer, vk_buffer_, region);
        });

        vmaDestroyBuffer(allocator, staging_buffer, staging_alloc);
    }

    descriptor_.buffer = vk_buffer_;
    descriptor_.offset = 0;
    descriptor_.range = size;
}

buffer::~buffer()
{
    vmaDestroyBuffer(ctx()->memory()->allocator(), vk_buffer_, allocation_);
}

std::tuple<vk::Buffer, VmaAllocation> buffer::allocate_vk_buffer(graphics* ctx,
    const void* data, u32 size, vk::BufferUsageFlags usage, bool mapped,
    VmaMemoryUsage memory_usage)
{
    vk::Buffer buffer;
    VmaAllocation allocation;

    auto allocator = ctx->memory()->allocator();

    VmaAllocationCreateInfo allocation_create_info{
        .flags{ VMA_ALLOCATION_CREATE_MAPPED_BIT },
        .usage{ memory_usage },
    };

    vk::BufferCreateInfo buffer_create_info{ {}, size, usage,
        vk::SharingMode::eExclusive };


    if (!success(vmaCreateBuffer(allocator,
            reinterpret_cast<VkBufferCreateInfo*>(&buffer_create_info),
            &allocation_create_info, reinterpret_cast<VkBuffer*>(&buffer), &allocation,
            nullptr))) {
        debug::fatal("Failed to create staging buffer!");
    }

    if (data && !mapped) {
        void* tmp{};
        if (!success(vmaMapMemory(allocator, allocation, &tmp))) {
            debug::fatal("Failed to map staging buffer!");
        }

        memcpy(tmp, data, u32(size));
        vmaUnmapMemory(allocator, allocation);
    }

    return std::make_tuple(buffer, allocation);
}


} // namespace bnr