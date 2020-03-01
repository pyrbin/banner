#include <banner/gfx/device.hpp>
#include <banner/gfx/memory.hpp>
#include <banner/util/debug.hpp>

namespace ban {
memory::memory(device* dev)
{
    // clang-format off
    VmaAllocatorCreateInfo info
    { 
        .physicalDevice = dev->get_gpu(),
        .device = dev->get()
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
} // namespace ban
