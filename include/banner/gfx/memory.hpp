#pragma once

#include <vk_mem_alloc.h>

#include <banner/gfx/device.hpp>

namespace ban {
struct memory
{
    using uptr = std::unique_ptr<memory>;

    explicit memory(device* device);
    virtual ~memory();

    VmaAllocator get() const { return vma_allocator_; }

private:
    VmaAllocator vma_allocator_;
};
} // namespace ban
