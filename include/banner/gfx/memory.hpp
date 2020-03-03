#pragma once

#include <vk_mem_alloc.h>

#include <banner/gfx/device.hpp>

namespace ban {
struct memory
{
    using ptr = memory*;
    using uptr = std::unique_ptr<memory>;

    explicit memory(device::ptr dev);
    virtual ~memory();

    VmaAllocator get() const { return vma_allocator_; }

private:
    VmaAllocator vma_allocator_;
};
} // namespace ban