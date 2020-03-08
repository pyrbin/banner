#pragma once

#include <array>

#include <banner/core/types.hpp>
#include <banner/gfx/res/buffer.hpp>
#include <banner/gfx/res/resource.hpp>
#include <vulkan/vulkan.hpp>

namespace bnr {
struct buffer;

struct vertex
{
    using list = vector<vertex>;

    v2 pos;
    v3 color;
};

using mesh_index = u32;

using mesh_indices = vector<mesh_index>;

struct mesh_primitive : resource
{
    struct data
    {
        vertex::list vertices;
        mesh_indices indices;

        bool has_vertices() const { return !vertices.empty(); }
        bool has_indices() const { return !indices.empty(); }

        u64 vertices_size() const { return vertices.size() * sizeof(vertex); }
        u64 indices_size() const { return indices.size() * sizeof(mesh_index); }
    };

    enum class type
    {
        triangle,
        cube,
        quad
    };

    explicit mesh_primitive(graphics* ctx, type mesh_type);

    virtual ~mesh_primitive() = default;

    auto ctx() { return ctx_; }

    auto size() const { return buffer_vertices_->size(); }

    auto empty() const { return data_.vertices.empty(); }

    void draw(vk::CommandBuffer buf) const;

private:
    static data make_mesh_data(type type);

    graphics* ctx_;
    type type_;
    data data_;

    uptr<buffer> buffer_vertices_;
    uptr<buffer> buffer_indices_;
};

sptr<mesh_primitive> make_triangle(graphics* ctx);

sptr<mesh_primitive> make_quad(graphics* ctx);
} // namespace bnr