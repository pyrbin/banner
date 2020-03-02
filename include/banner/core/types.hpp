#pragma once

#include <functional>

namespace ban {

using u64 = unsigned long long;
using u32 = unsigned int;
using u16 = unsigned short;

using i64 = signed long long;
using i32 = signed int;
using i16 = signed short;

using f64 = double;
using f32 = float;

template<typename T>
using fn = std::function<T>;

#define mut mutable

} // namespace ban
