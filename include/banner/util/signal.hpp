#pragma once

#include <banner/core/types.hpp>
#include <nano_function.hpp>
#include <nano_observer.hpp>
#include <nano_signal_slot.hpp>

namespace ban {
template<typename T>
using signal = Nano::Signal<T>;

template<typename T>
using observer = Nano::Observer<T>;
} // namespace ban