#include <stdarg.h>
#include <stdio.h>
#include <string>

#include "debug.h"

namespace tde {

// TODO: add colored outputs, use std::format instead

static void write_log(const char* prepend, const char* message, va_list args) { 
	vprintf((std::string(prepend) + message + "\n").c_str(), args);
}

void
debug::trace(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    write_log("[trace]: ", message, args);
    va_end(args);
}

void
debug::log(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    write_log("[log]: ", message, args);
    va_end(args);
}

void
debug::warn(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    write_log("[warn]: ", message, args);
    va_end(args);
}

void
debug::err(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    write_log("[err]: ", message, args);
    va_end(args);
}

void
debug::fatal(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    write_log("[fatal]: ", message, args);
    va_end(args);
}

}