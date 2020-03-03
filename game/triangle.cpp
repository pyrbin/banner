#include <iostream>
#include <time.h>

#include <banner/banner.hpp>

using namespace ban;

int main()
{
    platform* pl = new platform("BANNER");
    graphics* gr = new graphics(pl);
    renderer* re = new renderer(gr->get_swap());

    auto pass = new render_pass(gr->get_device());
    auto sub = new subpass();
    auto pipe = new pipeline(gr->get_device());

    pass->add_subpass(sub);
    sub->add_pipeline(pipe);

    auto vert_shader_module =
        vk_utils::load_shader("shaders/shader.vert.spv", gr->get_device()->get());
    auto frag_shader_module =
        vk_utils::load_shader("shaders/shader.frag.spv", gr->get_device()->get());

    pipe->add_vertex_stage("main", vert_shader_module);
    pipe->add_fragment_stage("main", frag_shader_module);
    pipe->on_process = [&](vk::CommandBuffer buff) { debug::log("buffer buffer buffer"); };

    pass->create(gr->get_swap());

    pipe->build(gr->get_swap()->get_extent(), pass->get());

    re->add_task([&](vk::CommandBuffer buff) {
        //////////////////////////////////////
        pass->process(re->get_current(), buff);
        //////////////////////////////////////
    });

    pl->on_update.connect([&]() {
        //////////////////////////////////////
        re->render();
        //////////////////////////////////////
    });

    pl->start_loop();
}