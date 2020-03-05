#include <banner/gfx/memory.hpp>
#include <banner/util/debug.hpp>

namespace bnr {
memory::memory(device* device)
{
    // clang-format off
    VmaAllocatorCreateInfo info
    {
        .physicalDevice = device->get_gpu(),
        .device = device->vk()
    };

    vmaCreateAllocator(&info, &vma_allocator_);
}

memory::~memory()
{
    if (vma_allocator_) {
        vmaDestroyAllocator(vma_allocator_);
        vma_allocator_ = nullptr;
    }
};
} // namespace bnr
