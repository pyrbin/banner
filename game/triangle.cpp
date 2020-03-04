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
        vk_utils::load_shader("shaders/shader.vert.spv", gr->get_swap()->get_device()->vk());
    auto frag_shader_module =
        vk_utils::load_shader("shaders/shader.frag.spv", gr->get_swap()->get_device()->vk());

    ////////////////////////////////////////////////

    render_pass* pass = new render_pass();
    {
        auto color_attach = vk::AttachmentDescription{ {}, gr->get_swap()->get_format().format,
            vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore, {}, {}, {}, vk::ImageLayout::ePresentSrcKHR };

        auto color_ref = vk::AttachmentReference{ 0, vk::ImageLayout::eColorAttachmentOptimal };

        auto sp = new subpass({ {}, vk::PipelineBindPoint::eGraphics,
            /*inAttachmentCount*/ 0, nullptr, 1, &color_ref });

        pass->add(color_attach);
        pass->add(sp);
        pass->create(gr->get_swap());
    }

    //////////////////////////////////////////////

    re->add_task([&](vk::CommandBuffer buff) { pass->process(re->get_current(), buff); });

    pl->on_update.connect([&]() { re->render(); });
    pl->start_loop();
}
