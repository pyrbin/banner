#pragma once

namespace tde {

struct debug
{
    static void trace(const char* message, ...);
    static void log(const char* message, ...);
    static void warn(const char* message, ...);
    static void err(const char* message, ...);
    static void fatal(const char* message, ...);
};

}  // namespace tde