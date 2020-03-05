#pragma once

#include <chrono>

#include <banner/core/types.hpp>

namespace bnr {

using clock = std::chrono::high_resolution_clock;
using duration = clock::duration;
using time_point = clock::time_point;

using sec = std::chrono::seconds;
using ms = std::chrono::milliseconds;

struct timer final
{
    timer()
        : start{ clock::now() }
    {}

    void restart() { start = clock::now(); }

    ms elapsed()
    {
        const auto now = clock::now();
        return std::chrono::duration_cast<ms>(now - start);
    }

private:
    time_point start;
};

template<typename T, typename D>
inline T duration_cast(D&& duration)
{
    return std::chrono::duration_cast<T>(duration);
}

inline sec to_sec(ms ms)
{
    return sec(i32(ms.count() / 1000.f));
}

inline ms to_ms(sec sec)
{
    return ms(i32(sec.count() * 1000.f));
}


} // namespace bnr
