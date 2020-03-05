#include <banner/banner.hpp>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

using namespace ban;

int main()
{
    window* wnd = new ban::window("My window :)", { 800, 600 }, "icon.png");


    graphics* gr = new graphics(wnd);

    renderer* re = new renderer(gr);

    vk::ShaderModule vert_shader_module =
        vk_utils::load_shader("shaders/shader.vert.spv", &gr->get_device()->vk());

    vk::ShaderModule frag_shader_module =
        vk_utils::load_shader("shaders/shader.frag.spv", &gr->get_device()->vk());

    ////////////////// RENDER PASS //////////////////////

    render_pass* pass = new render_pass(gr->get_swap());
    {
        pass->add(render_pass::attachment()
                      .set_op(vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore)
                      .set_layout({}, vk::ImageLayout::ePresentSrcKHR));

        auto sub = new subpass();
        auto pipe = new pipeline();

        sub->set_color_attachment({ 0, vk::ImageLayout::eColorAttachmentOptimal });

        pipe->add_color_blend_attachment();
        pipe->add_vertex("main", vert_shader_module);
        pipe->add_fragment("main", frag_shader_module);
        pipe->on_process([&](vk::CommandBuffer cmd_buf) { cmd_buf.draw(3, 1, 0, 0); });

        sub->add_pipeline(pipe);
        pass->add(sub);
        pass->create();
    }

    //////////////////////////////////////////////////

    re->add_task([&](vk::CommandBuffer buff) { pass->process(re->get_current(), buff); });
    wnd->on_update.connect([&]() { re->update(); });

    while (!wnd->should_close()) {
        wnd->update();
    }


    gr->get_device()->vk().destroyShaderModule(vert_shader_module);
    gr->get_device()->vk().destroyShaderModule(frag_shader_module);

    delete re;

    delete pass;

    delete gr;

    delete wnd;

    _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF);
}
