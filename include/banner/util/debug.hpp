#pragma once

#include <banner/core/types.hpp>

namespace ban {
struct debug
{
    static void trace(cstr message, ...);
    static void log(cstr message, ...);
    static void warn(cstr message, ...);
    static void err(cstr message, ...);
    static void fatal(cstr message, ...);
};
} // namespace ban