#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace ban {
struct device;

class memory
{
public:
    explicit memory(device* dev);
    virtual ~memory();

    VmaAllocator get() const { return vma_allocator_; }

private:
    VmaAllocator vma_allocator_;
};
} // namespace ban
