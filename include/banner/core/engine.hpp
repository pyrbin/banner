#include <banner/core/types.hpp>
#include <banner/entity/entity.hpp>
#include <banner/gfx/graphics.hpp>
#include <banner/gfx/renderer.hpp>
#include <banner/gfx/window.hpp>
#include <banner/util/signal.hpp>
#include <banner/util/time.hpp>

namespace bnr {
struct render_pass;

struct default_render_pass
{
    default_render_pass(graphics* ctx);
    ~default_render_pass();

    auto pass() { return render_pass_.get(); }

private:
    uptr<render_pass> render_pass_;
};

struct engine
{
    struct config
    {
        str name;
        uv2 window_size;
        str icon_path = "";
        ms timestep = ms(16);
        u32 world_size = 100000;
        bool fullscreen = false;
    };

    struct runtime
    {
        timer timer;
        ms offset{ 0 };
    };

    explicit engine(engine::config cfg);
    ~engine();

    auto window() { return window_.get(); }
    auto renderer() { return renderer_.get(); }
    auto graphics() { return graphics_.get(); }
    auto world() { return world_.get(); }
    auto default_pass() { return default_pass_->pass(); }

    const config cfg;
    engine::runtime runtime;

    void run();
    void stop();

    signal<void()> on_load;
    signal<void()> on_init;
    signal<void()> on_update;
    signal<void()> on_render;
    signal<void()> on_teardown;

private:
    void load();
    void init();
    bool handle_events();
    void update();
    void render();
    void teardown();

    bool stop_engine_{ false };

    uptr<bnr::window> window_;
    uptr<bnr::graphics> graphics_;
    uptr<bnr::renderer> renderer_;
    uptr<bnr::world> world_;
    uptr<bnr::default_render_pass> default_pass_;
};
} // namespace bnr
