/**
 * @file basic_cbs.hpp
 * @brief Declares a baseline optimal Conflict-Based Search solver.
 */
#pragma once

#include <memory>

#include "basic_cbs_options.hpp"
#include "skymapf/algorithms/low_level/i_single_agent_planner.hpp"
#include "skymapf/solver/i_solver.hpp"

namespace skymapf::algorithms::cbs {

class BasicCBSSolver final : public solver::ISolver {
public:
    using solver::ISolver::solve;

    explicit BasicCBSSolver(
        BasicCBSOptions options = {},
        std::unique_ptr<low_level::ISingleAgentPlanner> low_level_planner = nullptr
    );

    solver::SolverInfo info() const override;

    solver::SolveResult solve(
        const solver::SolveInstance& instance,
        const solver::SolveOptions& options
    ) override;

private:
    BasicCBSOptions options_;
    std::unique_ptr<low_level::ISingleAgentPlanner> low_level_planner_;
};

}  // namespace skymapf::algorithms::cbs
