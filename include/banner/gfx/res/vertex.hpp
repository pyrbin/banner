#pragma once

#include <banner/core/types.hpp>

namespace bnr {
struct vertex
{
    using list = vector<vertex>;

    v2 pos;
    v3 color;
};

} // namespace bnr
