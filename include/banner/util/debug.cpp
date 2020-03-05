#include <cstdarg>
#include <cstdio>
#include <string>

#include <banner/util/debug.hpp>

namespace ban {
// TODO: add colored outputs, use std::format instead

static void write_log(cstr prepend, cstr message, const va_list args)
{
    vprintf((std::string(prepend) + message + "\n").c_str(), args);
}

void debug::trace(cstr message, ...)
{
    va_list args;
    va_start(args, message);
    write_log("[trace]: ", message, args);
    va_end(args);
}

void debug::log(cstr message, ...)
{
    va_list args;
    va_start(args, message);
    write_log("[log]: ", message, args);
    va_end(args);
}

void debug::warn(cstr message, ...)
{
    va_list args;
    va_start(args, message);
    write_log("[warn]: ", message, args);
    va_end(args);
}

void debug::err(cstr message, ...)
{
    va_list args;
    va_start(args, message);
    write_log("[err]: ", message, args);
    va_end(args);
}

void debug::fatal(cstr message, ...)
{
    va_list args;
    va_start(args, message);
    write_log("[fatal]: ", message, args);
    va_end(args);
}
} // namespace ban