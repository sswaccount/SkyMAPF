/**
 * @file search_limits.hpp
 * @brief Defines generic runtime limits for search algorithms.
 */
#pragma once

#include <cstdint>

namespace skymapf::search {

struct SearchLimits {
    std::uint64_t time_limit_ms{0};
    std::uint64_t node_limit{0};
};

}  // namespace skymapf::search
