/**
 * @file metrics.hpp
 * @brief Defines solution quality and search effort metrics.
 */
#pragma once

#include <cstdint>

#include "plan.hpp"
#include "skymapf/common/ids.hpp"

namespace skymapf::solution {

/// Common MAPF solution metrics.
struct SolutionMetrics {
    std::uint64_t sum_of_costs{0};
    common::TimeStep makespan{0};
    std::uint64_t expanded_nodes{0};
    std::uint64_t generated_nodes{0};
};

SolutionMetrics compute_metrics(const Plan& plan);

}  // namespace skymapf::solution
