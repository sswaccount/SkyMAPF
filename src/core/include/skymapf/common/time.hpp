/**
 * @file time.hpp
 * @brief Defines basic utilities for discrete simulation time.
 */
#pragma once

#include "ids.hpp"

namespace skymapf::common {

/// Sentinel value representing an invalid or unknown time step.
constexpr TimeStep kInvalidTimeStep = static_cast<TimeStep>(-1);

/// Returns the next discrete time step.
inline TimeStep next_time(TimeStep t) noexcept {
    return static_cast<TimeStep>(t + 1);
}

}  // namespace skymapf::common
