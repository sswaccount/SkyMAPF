/**
 * @file prioritized_planning.hpp
 * @brief Declares a baseline prioritized planning MAPF solver.
 */
#pragma once

#include <memory>

#include "prioritized_options.hpp"
#include "skymapf/algorithms/low_level/i_single_agent_planner.hpp"
#include "skymapf/solver/i_solver.hpp"

namespace skymapf::algorithms::prioritized {

class PrioritizedPlanningSolver final : public solver::ISolver {
public:
    using solver::ISolver::solve;

    explicit PrioritizedPlanningSolver(
        PrioritizedPlanningOptions options = {},
        std::unique_ptr<low_level::ISingleAgentPlanner> low_level_planner = nullptr
    );

    solver::SolverInfo info() const override;

    solver::SolveResult solve(
        const solver::SolveInstance& instance,
        const solver::SolveOptions& options
    ) override;

private:
    PrioritizedPlanningOptions options_;
    std::unique_ptr<low_level::ISingleAgentPlanner> low_level_planner_;
};

}  // namespace skymapf::algorithms::prioritized
