#include <banner/gfx/graphics.hpp>
#include <banner/gfx/res/buffer.hpp>
#include <banner/gfx/vk_utils.hpp>

namespace bnr {
using vk_utils::success;

buffer::buffer(graphics* ctx, vk::BufferUsageFlagBits usage, const void* data, u32 size)
    : ctx_{ ctx }
{
    auto allocator = ctx_->memory()->allocator();

    vk::BufferCreateInfo create_info = { {}, size, usage, vk::SharingMode::eExclusive };

    VmaAllocationCreateInfo alloc_info{
        .flags{ VMA_ALLOCATION_CREATE_MAPPED_BIT },
        .usage{ VMA_MEMORY_USAGE_GPU_ONLY },
    };

    if (!success(vmaCreateBuffer(allocator,
            reinterpret_cast<VkBufferCreateInfo*>(&create_info), &alloc_info,
            reinterpret_cast<VkBuffer*>(&vk_buffer_), &allocation_, &allocation_info_))) {
        debug::fatal("Failed to create buffer!");
    }

    if (data) {
        void* mapping{};

        /*
        if (!vk_utils::success(vmaMapMemory(allocator, allocation_, &mapped))) {
            debug::fatal("Failed to map buffer!");
            return;
        }

        memcpy(mapped, data, u32(size));
        vmaUnmapMemory(ctx_->memory()->allocator(), allocation_);
        */
    }

    descriptor_.buffer = vk_buffer_;
    descriptor_.offset = 0;
    descriptor_.range = size;
}

buffer::~buffer()
{
    vmaDestroyBuffer(ctx()->memory()->allocator(), vk_buffer_, allocation_);
}

void buffer::draw(vk::CommandBuffer buffer, const vector<vertex>& vertices)
{
    buffer.bindVertexBuffers(0, vk(), vk::DeviceSize(0));
    buffer.draw(u32(vertices.size()), 1, 0, 0);
}
} // namespace bnr