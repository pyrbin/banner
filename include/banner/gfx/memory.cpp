#include <banner/gfx/memory.hpp>
#include <banner/gfx/device.hpp>

namespace ban {
memory::memory(device* dev)
{
    VmaAllocatorCreateInfo info;
    info.device = dev->get();
    info.physicalDevice = dev->get_gpu();
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
