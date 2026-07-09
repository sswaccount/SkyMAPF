/**
 * @file search_stats.hpp
 * @brief Defines generic search effort counters.
 */
#pragma once

#include <cstdint>

namespace skymapf::search {

struct SearchStats {
    std::uint64_t expanded_nodes{0};
    std::uint64_t generated_nodes{0};
};

}  // namespace skymapf::search
