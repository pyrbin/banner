#pragma once
#include <cstdlib>
#include <type_traits>

namespace bnr {
template<typename T>
std::enable_if_t<std::is_arithmetic_v<T>, T> rnd(T lo, T hi)
{
    return lo + static_cast<T>(rand()) / (static_cast<T>(RAND_MAX / (hi - lo)));
}
}
