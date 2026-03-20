#pragma once

#include "ids.hpp"

namespace skymapf::common {

constexpr TimeStep kInvalidTimeStep = static_cast<TimeStep>(-1);

inline TimeStep next_time(TimeStep t) noexcept {
    return static_cast<TimeStep>(t + 1);
}

}  // namespace skymapf::common
