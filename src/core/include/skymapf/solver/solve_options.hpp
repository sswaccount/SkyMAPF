/**
 * @file solve_options.hpp
 * @brief Defines common runtime options passed to solvers.
 */
#pragma once

#include <cstdint>

namespace skymapf::solver {

/// Common runtime options passed to solver executions.
struct SolveOptions {
    std::uint64_t time_limit_ms{0};
    std::uint64_t random_seed{0};
    std::uint64_t node_limit{0};
};

}  // namespace skymapf::solver
