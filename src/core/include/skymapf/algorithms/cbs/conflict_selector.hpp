/**
 * @file conflict_selector.hpp
 * @brief Declares deterministic conflict selection policies for CBS-family solvers.
 */
#pragma once

#include "skymapf/search/conflict.hpp"
#include "skymapf/solution/plan.hpp"

namespace skymapf::algorithms::cbs {

/// Baseline policies that can later be extended with cardinality-based ranking.
enum class ConflictSelectionStrategy {
    FirstDetected,
    Earliest,
};

class ConflictSelector {
public:
    static bool select(
        const solution::Plan& plan,
        ConflictSelectionStrategy strategy,
        search::Conflict& selected
    );
};

}  // namespace skymapf::algorithms::cbs
