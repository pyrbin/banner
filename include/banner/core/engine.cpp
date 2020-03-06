#include <banner/core/engine.hpp>
#include <banner/gfx/render_pass.hpp>

namespace bnr {
default_render_pass::default_render_pass(graphics* gfx)
    : render_pass_{ make_uptr<render_pass>(gfx) }
{
    render_pass_->add(
        render_pass::attachment()
            .set_op(vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore)
            .set_stencil_op(
                vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare)
            .set_layout({}, vk::ImageLayout::ePresentSrcKHR));

    auto sub = new subpass();
    sub->set_color_attachment({ 0, vk::ImageLayout::eColorAttachmentOptimal });
    // sub.set_depth_stencil_attachment({ 1,
    // vk::ImageLayout::eDepthStencilAttachmentOptimal });
    render_pass_->add(sub);

    render_pass_->add(subpass::dependency(VK_SUBPASS_EXTERNAL, 0)
                          .set_stage_mask(vk::PipelineStageFlagBits::eBottomOfPipe,
                              vk::PipelineStageFlagBits::eColorAttachmentOutput)
                          .set_access_mask(vk::AccessFlagBits::eMemoryRead,
                              vk::AccessFlagBits::eColorAttachmentRead |
                                  vk::AccessFlagBits::eColorAttachmentWrite));

    render_pass_->add(
        subpass::dependency(0, VK_SUBPASS_EXTERNAL)
            .set_stage_mask(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::PipelineStageFlagBits::eBottomOfPipe)
            .set_access_mask(vk::AccessFlagBits::eColorAttachmentRead |
                    vk::AccessFlagBits::eColorAttachmentWrite,
                vk::AccessFlagBits::eMemoryRead));

    render_pass_->create();
}

default_render_pass::~default_render_pass() {}

engine::engine(engine::config cfg)
    : cfg{ cfg }
{

    window_ =
        make_uptr<bnr::window>(cfg.name, cfg.window_size, cfg.icon_path, cfg.fullscreen);
    graphics_ = make_uptr<bnr::graphics>(window_.get());
    renderer_ = make_uptr<bnr::renderer>(graphics_.get());
    default_pass_ = make_uptr<bnr::default_render_pass>(graphics_.get());
    world_ = make_uptr<bnr::world>(cfg.world_size);
}

engine::~engine()
{
    /* Free renderer */
    renderer_.reset();
    /* Free render passes */
    default_pass_.reset();
    /* Free graphics context*/
    graphics_.reset();
    /* Free window/input related*/
    window_.reset();
    /* Rest ... */
    world_.reset();
}

void engine::run()
{
    load();
    init();
    update();
}

void engine::load()
{


    renderer_->add_task([&](vk::CommandBuffer buffer) {
        default_pass_->pass()->process(renderer_->current_index(), buffer);
    });

    window_->on_render.connect<&bnr::renderer::render>(*renderer_.get());

    on_load.fire();
}

void engine::init()
{
    on_init.fire();

    runtime.timer.restart();
    update();
    if (stop_engine_) {
        teardown();
    }
}

void engine::update()
{
    // todo: implement better game loop
    // http://www.fabiensanglard.net/timer_and_framerate/index.php
    // https://gafferongames.com/post/fix_your_timestep/
    for (;;) {
        auto& [timer, offset] = runtime;

        offset += timer.elapsed();
        timer.restart();

        if (stop_engine_ = handle_events()) {
            break;
        }

        while (offset >= cfg.timestep) {
            offset -= cfg.timestep;
            world_->update();
            on_update.fire();
        }

        auto alpha = (f64)offset.count() / cfg.timestep.count();
        render();
    }
}

bool engine::handle_events()
{
    window_->handle_events();

    return window_->should_close();
}

void engine::render()
{
    window_->render();
    on_render.fire();
}

void engine::teardown()
{
    on_teardown.fire();
}
} // namespace bnr
