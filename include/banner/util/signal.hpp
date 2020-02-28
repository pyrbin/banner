#pragma once

#include <nano_function.hpp>
#include <nano_signal_slot.hpp>
#include "nano_observer.hpp"

namespace ban {
template <typename T>
using signal = Nano::Signal<T>;

template <typename T>
using observer = Nano::Observer<T>;
} // namespace ban