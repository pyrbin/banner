#pragma once

namespace ban {
struct debug
{
    static void trace(const char* message, ...);
    static void log(const char* message, ...);
    static void warn(const char* message, ...);
    static void err(const char* message, ...);
    static void fatal(const char* message, ...);
};
} // namespace ban