#include "skymapf/algorithms/prioritized/prioritized_planning.hpp"

#include <chrono>
#include <memory>
#include <utility>

#include "skymapf/algorithms/low_level/space_time_astar.hpp"
#include "skymapf/search/constraint_table.hpp"
#include "skymapf/search/reservation_table.hpp"
#include "skymapf/solution/metrics.hpp"
#include "skymapf/solution/validation.hpp"

namespace skymapf::algorithms::prioritized {

PrioritizedPlanningSolver::PrioritizedPlanningSolver(
    PrioritizedPlanningOptions options,
    std::unique_ptr<low_level::ISingleAgentPlanner> low_level_planner
)
    : options_(std::move(options)),
      low_level_planner_(
          low_level_planner
              ? std::move(low_level_planner)
              : std::make_unique<low_level::SpaceTimeAStar>()
      ) {}

solver::SolverInfo PrioritizedPlanningSolver::info() const {
    return solver::SolverInfo{
        "prioritized_planning",
        solver::SolverFamily::SearchBased,
        "0.0.1-beta",
        "Baseline prioritized planning with a reusable low-level planner.",
        {}
    };
}

solver::SolveResult PrioritizedPlanningSolver::solve(
    const solver::SolveInstance& solve_instance,
    const solver::SolveOptions& solve_options
) {
    const auto started = std::chrono::steady_clock::now();
    solver::SolveResult result;

    search::ReservationTable reservations;
    solution::Plan plan;

    auto priority_order = options_.priority_order;
    if (priority_order.empty()) {
        for (const auto& entry : solve_instance.instance.task().agent_entries()) {
            priority_order.push_back(entry.agent_id);
        }
    }

    auto low_level_options = options_.low_level;
    if (solve_options.node_limit > 0) {
        low_level_options.node_limit = solve_options.node_limit;
    }

    search::ConstraintTable constraints;
    for (const auto agent_id : priority_order) {
        const auto low_result = low_level_planner_->find_path(
            solve_instance.instance,
            agent_id,
            constraints,
            reservations,
            low_level_options
        );
        result.metrics.expanded_nodes += low_result.stats.expanded_nodes;
        result.metrics.generated_nodes += low_result.stats.generated_nodes;

        if (low_result.status != low_level::LowLevelStatus::Success) {
            result.status = low_result.status == low_level::LowLevelStatus::NodeLimit
                ? solver::SolveStatus::Timeout
                : solver::SolveStatus::Infeasible;
            result.message = low_result.message;
            return result;
        }

        reservations.reserve_path(low_result.path, options_.reserve_goals_forever);
        plan.agent_paths.push_back(low_result.path);
    }

    const auto validation = solution::SolutionValidator::validate(
        solve_instance.instance,
        plan
    );
    if (!validation.valid()) {
        result.status = solver::SolveStatus::Error;
        result.message = "prioritized planning produced an invalid plan";
        result.plan = std::move(plan);
        return result;
    }

    result.status = solver::SolveStatus::Success;
    result.plan = std::move(plan);
    const auto metrics = solution::compute_metrics(result.plan);
    result.metrics.sum_of_costs = metrics.sum_of_costs;
    result.metrics.makespan = metrics.makespan;
    result.elapsed_ms = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - started
        ).count()
    );
    return result;
}

}  // namespace skymapf::algorithms::prioritized
