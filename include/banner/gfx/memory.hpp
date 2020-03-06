#pragma once

#include <vk_mem_alloc.h>

#include <banner/gfx/device.hpp>

namespace bnr {
struct memory
{
    explicit memory(device* device);
    ~memory();

    VmaAllocator allocator() const { return vma_allocator_; }

private:
    VmaAllocator vma_allocator_;
};
} // namespace bnr
