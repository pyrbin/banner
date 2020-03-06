#include <banner/banner.hpp>
#include <banner/util/debug.hpp>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

using namespace bnr;

struct example_system
{
    void update(v2& pos) const {}
};

int main()
{
    engine* engine;

    {
        engine = new bnr::engine({ "(:^o)-|--<", { 800, 600 }, "icon.png" });
        auto pipeline = new bnr::pipeline();

        engine->on_init = [&]() {
            // Insert system
            engine->world()->create<v2>();
            engine->world()->insert<example_system>();

            // Init triangle pipeline
            auto ctx = engine->graphics();
            pipeline->add_color_blend_attachment();
            pipeline->add_vertex("main", ctx->load_shader("shaders/shader.vert.spv"));
            pipeline->add_fragment("main", ctx->load_shader("shaders/shader.frag.spv"));
            pipeline->on_process = [&](vk::CommandBuffer cmd_buf) {
                cmd_buf.draw(3, 1, 0, 0);
            };
            engine->default_pass()->add(pipeline);
        };

        engine->run();
    }

    delete engine;

    _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF);
}
