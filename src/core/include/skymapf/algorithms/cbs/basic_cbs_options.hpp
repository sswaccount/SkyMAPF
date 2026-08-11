/**
 * @file basic_cbs_options.hpp
 * @brief Defines options for the baseline Conflict-Based Search solver.
 */
#pragma once

#include <cstdint>

#include "conflict_selector.hpp"
#include "skymapf/algorithms/low_level/low_level_options.hpp"

namespace skymapf::algorithms::cbs {

struct BasicCBSOptions {
    std::uint64_t high_level_node_limit{0};
    ConflictSelectionStrategy conflict_selection{ConflictSelectionStrategy::Earliest};
    low_level::LowLevelSearchOptions low_level;
};

}  // namespace skymapf::algorithms::cbs
