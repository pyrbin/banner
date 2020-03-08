#include <banner/gfx/graphics.hpp>
#include <banner/gfx/res/mesh.hpp>

namespace bnr {
mesh_primitive::mesh_primitive(graphics* ctx, type mesh_type)
    : ctx_{ ctx }
    , type_{ mesh_type }
    , data_{ mesh_primitive::make_mesh_data(type_) }

{
    if (data_.has_vertices()) {
        buffer_vertices_ =
            std::make_unique<buffer>(ctx_, data_.vertices.data(), data_.vertices_size());
    }
    if (data_.has_indices()) {
        buffer_indices_ = std::make_unique<buffer>(ctx_, data_.indices.data(),
            data_.indices_size(), vk::BufferUsageFlagBits::eIndexBuffer);
    }
}

void mesh_primitive::draw(vk::CommandBuffer buffer) const
{
    if (buffer_vertices_ && buffer_vertices_->valid()) {
        buffer.bindVertexBuffers(0, buffer_vertices_->vk(), vk::DeviceSize(0));
    }

    if (buffer_indices_ && buffer_indices_->valid()) {
        buffer.bindIndexBuffer(
            buffer_indices_->vk(), vk::DeviceSize(0), vk::IndexType::eUint32);
    }

    if (data_.has_indices()) {
        buffer.drawIndexed(u32(data_.indices.size()), 1, 0, 0, 0);
    } else {
        buffer.draw(u32(data_.vertices.size()), 1, 0, 0);
    }
}

mesh_primitive::data mesh_primitive::make_mesh_data(type type)
{
    vertex::list vertices;
    mesh_indices indices;

    switch (type) {
    case mesh_primitive::type::triangle: {
        vertices = { { { 0.0f, -0.5f }, { 1.0f, 1.0f, 1.0f } },
            { { 0.5f, 0.5f }, { 0.0f, 1.0f, 0.0f } },
            { { -0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } } };
    }
    case mesh_primitive::type::quad: {
        vertices = { { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
            { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
            { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
            { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } } };
        indices = { 0, 1, 2, 2, 3, 0 };
    }
    default:
        break;
    }

    return { vertices, indices };
}

sptr<mesh_primitive> make_triangle(graphics* ctx)
{
    return std::make_shared<mesh_primitive>(ctx, mesh_primitive::type::triangle);
}

sptr<mesh_primitive> make_quad(graphics* ctx)
{
    return std::make_shared<mesh_primitive>(ctx, mesh_primitive::type::quad);
}
} // namespace bnr