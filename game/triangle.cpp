#include <iostream>
#include <time.h>

#include <banner/banner.hpp>

using namespace ban;

int main()
{
    platform* pl = new platform("BANNER");
    graphics* gr = new graphics(pl);
    renderer* re = new renderer(gr->get_swap());

    auto vert_shader_module =
        vk_utils::load_shader("shaders/shader.vert.spv", gr->get_swap()->get_device()->get());
    auto frag_shader_module =
        vk_utils::load_shader("shaders/shader.frag.spv", gr->get_swap()->get_device()->get());

    re->add_task([&](vk::CommandBuffer buff) {});

    pl->on_update.connect([&]() { re->render(); });
    pl->start_loop();
}
