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
        vk_utils::load_shader("shaders/shader.vert.spv", &gr->get_swap()->get_device()->vk());
    auto frag_shader_module =
        vk_utils::load_shader("shaders/shader.frag.spv", &gr->get_swap()->get_device()->vk());

    ////////////////////////////////////////////////

    render_pass* pass = new render_pass(gr->get_swap());
    {

        pass->add(render_pass::attachment()
                      .set_op(vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore)
                      .set_layout({}, vk::ImageLayout::ePresentSrcKHR));

        auto sub = new subpass();
        auto pipe = new pipeline();

        sub->set_color_attachment({ 0, vk::ImageLayout::eColorAttachmentOptimal });

        pipe->add_color_blend_attachment();
        pipe->add_fragment("main", frag_shader_module);
        pipe->add_vertex("main", vert_shader_module);

        sub->add_pipeline(pipe);

        pass->add(sub);

        pass->add(render_pass::dependency()
                      .set_subpass(VK_SUBPASS_EXTERNAL, 0)
                      .set_stage_mask(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                          vk::PipelineStageFlagBits::eColorAttachmentOutput)
                      .set_access_mask({},
                          vk::AccessFlagBits::eColorAttachmentRead |
                              vk::AccessFlagBits::eColorAttachmentWrite));

        pass->create();
    }

    //////////////////////////////////////////////

    re->add_task([&](vk::CommandBuffer buff) { pass->process(re->get_current(), buff); });

    pl->on_update.connect([&]() { re->render(); });
    pl->start_loop();
}
