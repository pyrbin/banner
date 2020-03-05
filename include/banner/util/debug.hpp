#pragma once

#include <banner/core/types.hpp>

namespace bnr {
struct debug
{
    static void trace(cstr message, ...);
    static void log(cstr message, ...);
    static void warn(cstr message, ...);
    static void err(cstr message, ...);
    static void fatal(cstr message, ...);
};
} // namespace bnr
