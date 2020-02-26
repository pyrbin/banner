#include "render/vulkan_graphics.hpp"
#include "render/vulkan_utils.hpp"
#include "debug.hpp"
#include <vector>
#include <memory>
#include <vector>
#include "common/types.hpp"

namespace tde {

struct renderer
{
    renderer(vulkan_graphics* graphics) : _graphics {graphics}
    {
        for (size_t i = 0; i < graphics->swapchain_imageviews().size(); i++) {
            vk::FenceCreateInfo info = {};
            info.flags = vk::FenceCreateFlagBits::eSignaled;
            _fences.push_back(_graphics->device()->createFence(info));
        }

        _sync = {
            _graphics->device()->createSemaphore({}), _graphics->device()->createSemaphore({})};
    }

    u32 get_index() const { return _index; }

    void submit(const vk::CommandBuffer& _command_buffer)
    {
        _command_buffers.push_back(_command_buffer);
    }

    void wait() const { _graphics->device()->waitForFences(_fences, true, u64(-1)); }

    void pre_update(const float& dt)
    {
        _index = _graphics->device()
                     ->acquireNextImageKHR(
                         *_graphics->swapchain(), UINT32_MAX, _sync.lock_render, nullptr)
                     .value;
        debug::log(" index: , %d", _index);
        _graphics->device()->waitForFences(_fences[_index], true, UINT32_MAX);
        _graphics->device()->resetFences(_fences[_index]);
    }

    void update(const float& dt)
    {
        vk::Semaphore wait_semas[] = {_sync.lock_render};
        vk::Semaphore sig_semas[] = {_sync.lock_present};
        vk::PipelineStageFlags stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        vk::SubmitInfo submit_info(
            u32(1),
            wait_semas,
            stages,
            u32(_command_buffers.size()),
            (vk::CommandBuffer*)_command_buffers.data(),
            u32(1),
            sig_semas);

        _graphics->graphicsQueue()->submit(submit_info, _fences[_index]);

        vk::PresentInfoKHR present_info {
            u32(1), sig_semas, u32(1), {_graphics->swapchain()}, {&_index}};

        _graphics->presentQueue()->presentKHR(present_info);
    }
    vulkan_graphics* _graphics;
    std::vector<vk::Fence> _fences;
    vulkan_utils::synchronization _sync;
    std::vector<std::reference_wrapper<const vk::CommandBuffer>> _command_buffers;
    u32 _index;
};

} // namespace tde
