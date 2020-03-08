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
        bnr::buffer* buffer;

        auto pipeline = new bnr::pipeline();

        engine->on_init = [&]() {
            // Insert system
            engine->world()->create<v2>();
            engine->world()->insert<example_system>();

            // Init triangle pipeline
            auto ctx = engine->graphics();
            pipeline->add_color_blend_attachment();
            pipeline->set_vertex_input_bindings(
                { { 0, sizeof(vertex), vk::VertexInputRate::eVertex } });

            pipeline->set_vertex_input_attributes(
                { { 0u, 0u, vk::Format::eR32G32Sfloat, offsetof(vertex, pos) },
                    { 1u, 0u, vk::Format::eR32G32B32Sfloat, offsetof(vertex, color) } });

            pipeline->add_vertex_shader(
                "main", ctx->load_shader("shaders/shader.vert.spv"));

            pipeline->add_fragment_shader(
                "main", ctx->load_shader("shaders/shader.frag.spv"));

            // Create buffer
            const std::vector<vertex> vertices{ { { 0.0f, -0.5f }, { 1.0f, 1.0f, 1.0f } },
                { { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
                { { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } } };

            buffer = new bnr::buffer(
                ctx, (bytes)vertices.data(), u32(vertices.size() * sizeof(vertex)));

            pipeline->on_process = [&, buffer](vk::CommandBuffer cmd_buf) {
                buffer->draw(cmd_buf);
            };

            engine->default_pass()->add(pipeline);
        };

        engine->on_teardown = [&]() { delete buffer; };

        engine->run();
    }

    delete engine;

    _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF);
}