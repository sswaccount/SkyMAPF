/**
 * @file solve_result.hpp
 * @brief Defines standardized solver outputs.
 */
#pragma once

#include <cstdint>
#include <string>

#include "skymapf/solution/metrics.hpp"
#include "skymapf/solution/plan.hpp"

namespace skymapf::solver {

/// Standardized solve status outcomes.
enum class SolveStatus {
    Success,
    Infeasible,
    Timeout,
    Unsupported,
    Error,
};

/// Output bundle for one solver invocation.
struct SolveResult {
    SolveStatus status{SolveStatus::Error};
    solution::Plan plan;
    solution::SolutionMetrics metrics;
    std::uint64_t elapsed_ms{0};
    std::string message;
};

}  // namespace skymapf::solver
