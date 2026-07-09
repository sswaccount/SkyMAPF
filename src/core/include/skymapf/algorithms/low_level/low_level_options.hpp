/**
 * @file low_level_options.hpp
 * @brief Defines low-level single-agent search options.
 */
#pragma once

#include <cstdint>

#include "skymapf/common/ids.hpp"

namespace skymapf::algorithms::low_level {

struct LowLevelSearchOptions {
    common::TimeStep max_time{256};
    std::uint64_t node_limit{0};
};

}  // namespace skymapf::algorithms::low_level
