#pragma once

#include <banner/util/debug.hpp>

#if _WIN32 || _WIN64
#define TARGET_WIN
#elif __linux__
#define TARGET_LIN
#elif __APPLE__
#define TARGET_MAC
#else
#error "Unable to determine platform"
#endif

// Assertion
#define ASSERTS_ENABLED
#ifdef ASSERTS_ENABLED

#include <iostream>

#if _MSC_VER
#include <intrin.h>
#define DEBUG_BREAK() __debugbreak();
#else
#define DEBUG_BREAK() __asm { int 3 }
#endif

// TODO: add platform specific force inline
__forceinline void
__report_assert_failed(const char* expr, const char* message, const char* file, int line)
{
    std::cerr << "Assertion Failure: " << expr << ", message: '" << message
              << "', in file: " << file << ", line: " << line << "\n";
}

#define ASSERT(expr) \
    { \
        if (expr) { \
        } else { \
            __report_assert_failed(#expr, "", __FILE__, __LINE__); \
            DEBUG_BREAK(); \
        } \
    }

#define ASSERT_MSG(expr, message) \
    { \
        if (expr) { \
        } else { \
            __report_assert_failed(#expr, message, __FILE__, __LINE__); \
            DEBUG_BREAK(); \
        } \
    }

#endif
