#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm\ext\vector_int2_sized.hpp>
#include <glm\ext\vector_int3_sized.hpp>
#include <glm\ext\vector_int4_sized.hpp>
#include <glm\ext\vector_uint2_sized.hpp>
#include <glm\ext\vector_uint3_sized.hpp>
#include <glm\ext\vector_uint4_sized.hpp>

namespace bnr {
using u64 = unsigned long long;
using u32 = unsigned int;
using u16 = unsigned short;

using i64 = signed long long;
using i32 = signed int;
using i16 = signed short;

using f64 = double;
using f32 = float;

using c8 = char;
using uc8 = unsigned char;

using bytes = uc8*;
using cstr = const c8*;
using str = std::string;
using str_ref = const str&;

template<typename T>
using uptr = std::unique_ptr<T>;

template<typename T>
using sptr = std::shared_ptr<T>;

template<typename... T>
using vector = std::vector<T...>;

template<typename T>
using fn = std::function<T>;

/*
    math
*/

using glm::vec2;
using glm::vec3;
using glm::vec4;

using v2 = vec2;
using v3 = vec3;
using v4 = vec4;

using glm::u32vec2;
using glm::u32vec3;
using glm::u32vec4;

using uv2 = u32vec2;
using uv3 = u32vec3;
using uv4 = u32vec4;

using glm::i32vec2;
using glm::i32vec3;
using glm::i32vec4;

using iv2 = i32vec2;
using iv3 = i32vec3;
using iv4 = i32vec4;

template<typename T>
inline auto to_str(T&& t)
{
    return std::to_string(t);
}

template<typename T, typename... Args>
inline auto make_uptr(Args&&... args) -> std::unique_ptr<T>
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

} // namespace bnr
