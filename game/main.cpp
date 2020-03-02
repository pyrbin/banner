#include <iostream>
#include <time.h>

#include <banner/banner.hpp>

using namespace ban;

int main()
{
    platform* pl = new platform("BANNER");
    graphics* gr = new graphics(pl);

    world* wo = new world(2000);

    renderer* rend = new renderer(gr->get_swap());

    srand(time(nullptr));

    vk::ClearColorValue clear_values = { std::array<float, 4>{
        rnd(0.f, 1.f), rnd(0.f, 1.f), rnd(0.f, 1.f), 1.f } };

    rend->add_task([&](vk::CommandBuffer buff) {
        // Random color

        vk::ImageSubresourceRange image_range{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

        auto image = rend->get_swap()->get_data().images[rend->get_current()];

        vk::ImageMemoryBarrier image_barrier{ vk::AccessFlagBits::eTransferWrite,
            vk::AccessFlagBits::eMemoryRead, vk::ImageLayout::eTransferDstOptimal,
            vk::ImageLayout::ePresentSrcKHR,
            rend->get_swap()->get_device()->get_queues().present_index,
            rend->get_swap()->get_device()->get_queues().present_index, image, image_range };

        buff.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eTransfer, (vk::DependencyFlagBits)0, {}, {}, image_barrier);

        buff.clearColorImage(
            image, vk::ImageLayout::eTransferDstOptimal, &clear_values, 1, &image_range);

        buff.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eBottomOfPipe, (vk::DependencyFlagBits)0, {}, {},
            image_barrier);
    });

    pl->on_update.connect([&]() {
        wo->update();
        rend->render();
    });

    pl->start_loop();
}