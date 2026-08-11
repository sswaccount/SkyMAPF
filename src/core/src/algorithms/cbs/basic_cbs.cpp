#include "skymapf/algorithms/cbs/basic_cbs.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <queue>
#include <utility>
#include <vector>

#include "skymapf/algorithms/low_level/space_time_astar.hpp"
#include "skymapf/search/conflict_detector.hpp"
#include "skymapf/search/constraint.hpp"
#include "skymapf/search/constraint_table.hpp"
#include "skymapf/solution/metrics.hpp"
#include "skymapf/task/policy.hpp"

namespace skymapf::algorithms::cbs {

namespace {

struct CBSNode {
    std::uint64_t id{0};
    std::vector<search::Constraint> constraints;
    solution::Plan plan;
    std::uint64_t cost{0};
};

struct CompareNode {
    bool operator()(const std::shared_ptr<CBSNode>& lhs,
                    const std::shared_ptr<CBSNode>& rhs) const noexcept {
        if (lhs->cost == rhs->cost) {
            return lhs->id > rhs->id;
        }
        return lhs->cost > rhs->cost;
    }
};

bool supports_standard_mapf(const instance::InstanceModel& instance) {
    for (const auto& entry : instance.task().agent_entries()) {
        if (entry.start_time != 0 ||
            entry.goal_behavior != task::GoalArrivalBehavior::StayAtGoal ||
            entry.visit_sequence.checkpoints.size() != 2) {
            return false;
        }
    }
    return true;
}

search::ConstraintTable constraints_for(
    const std::vector<search::Constraint>& constraints,
    common::AgentId agent_id
) {
    search::ConstraintTable table;
    for (const auto& constraint : constraints) {
        if (constraint.agent_id == agent_id) {
            table.add(constraint);
        }
    }
    return table;
}

search::Constraint branch_constraint(
    const search::Conflict& conflict,
    common::AgentId agent_id
) {
    if (conflict.kind == search::ConflictKind::Vertex) {
        return search::Constraint::vertex(agent_id, conflict.cell, conflict.time);
    }
    if (agent_id == conflict.a1) {
        return search::Constraint::edge(
            agent_id,
            conflict.a1_from,
            conflict.a1_to,
            conflict.time
        );
    }
    return search::Constraint::edge(
        agent_id,
        conflict.a2_from,
        conflict.a2_to,
        conflict.time
    );
}

bool limit_reached(
    const std::chrono::steady_clock::time_point started,
    std::uint64_t time_limit_ms
) {
    if (time_limit_ms == 0) {
        return false;
    }
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - started
    ).count();
    return elapsed >= static_cast<std::int64_t>(time_limit_ms);
}

}  // namespace

BasicCBSSolver::BasicCBSSolver(
    BasicCBSOptions options,
    std::unique_ptr<low_level::ISingleAgentPlanner> low_level_planner
)
    : options_(std::move(options)),
      low_level_planner_(
          low_level_planner
              ? std::move(low_level_planner)
              : std::make_unique<low_level::SpaceTimeAStar>()
      ) {}

solver::SolverInfo BasicCBSSolver::info() const {
    return solver::SolverInfo{
        "basic_cbs",
        solver::SolverFamily::SearchBased,
        "0.0.1-beta",
        "Baseline optimal CBS for Standard MAPF v1.",
        {}
    };
}

solver::SolveResult BasicCBSSolver::solve(
    const solver::SolveInstance& solve_instance,
    const solver::SolveOptions& solve_options
) {
    const auto started = std::chrono::steady_clock::now();
    solver::SolveResult result;
    const auto& instance = solve_instance.instance;

    if (!supports_standard_mapf(instance)) {
        result.status = solver::SolveStatus::Unsupported;
        result.message = "basic CBS supports only Standard MAPF v1: t=0, start-goal, stay-at-goal";
        return result;
    }

    auto root = std::make_shared<CBSNode>();
    search::ConstraintTable empty_constraints;
    search::ReservationTable empty_reservations;
    for (const auto& entry : instance.task().agent_entries()) {
        const auto low_result = low_level_planner_->find_path(
            instance,
            entry.agent_id,
            empty_constraints,
            empty_reservations,
            options_.low_level
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
        root->plan.agent_paths.push_back(low_result.path);
    }
    root->cost = solution::compute_metrics(root->plan).sum_of_costs;

    std::priority_queue<
        std::shared_ptr<CBSNode>,
        std::vector<std::shared_ptr<CBSNode>>,
        CompareNode
    > open;
    open.push(root);
    std::uint64_t next_node_id = 1;
    std::uint64_t expanded_high_level = 0;

    const auto high_level_limit = solve_options.node_limit > 0
        ? solve_options.node_limit
        : options_.high_level_node_limit;

    while (!open.empty()) {
        if (limit_reached(started, solve_options.time_limit_ms)) {
            result.status = solver::SolveStatus::Timeout;
            result.message = "CBS time limit reached";
            return result;
        }
        if (high_level_limit > 0 && expanded_high_level >= high_level_limit) {
            result.status = solver::SolveStatus::Timeout;
            result.message = "CBS high-level node limit reached";
            return result;
        }

        auto node = open.top();
        open.pop();
        ++expanded_high_level;

        search::Conflict conflict;
        if (!search::ConflictDetector::first_conflict(node->plan, conflict)) {
            result.status = solver::SolveStatus::Success;
            result.plan = std::move(node->plan);
            result.metrics.expanded_nodes += expanded_high_level;
            result.metrics.generated_nodes += next_node_id;
            return result;
        }

        for (const auto agent_id : {conflict.a1, conflict.a2}) {
            auto child = std::make_shared<CBSNode>(*node);
            child->id = next_node_id++;
            child->constraints.push_back(branch_constraint(conflict, agent_id));

            const auto table = constraints_for(child->constraints, agent_id);
            const auto low_result = low_level_planner_->find_path(
                instance,
                agent_id,
                table,
                empty_reservations,
                options_.low_level
            );
            result.metrics.expanded_nodes += low_result.stats.expanded_nodes;
            result.metrics.generated_nodes += low_result.stats.generated_nodes;
            if (low_result.status != low_level::LowLevelStatus::Success) {
                continue;
            }

            auto* path = child->plan.find_path(agent_id);
            if (!path) {
                continue;
            }
            *path = low_result.path;
            child->cost = solution::compute_metrics(child->plan).sum_of_costs;
            open.push(std::move(child));
        }
    }

    result.status = solver::SolveStatus::Infeasible;
    result.message = "CBS exhausted the constraint tree";
    return result;
}

}  // namespace skymapf::algorithms::cbs
