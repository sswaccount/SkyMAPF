#include "skymapf/algorithms/low_level/space_time_astar.hpp"

#include <algorithm>
#include <cstdint>
#include <queue>
#include <unordered_map>
#include <utility>

namespace skymapf::algorithms::low_level {

namespace {

struct State {
    common::CellIndex cell{0};
    common::TimeStep time{0};

    bool operator==(const State& other) const noexcept {
        return cell == other.cell && time == other.time;
    }
};

struct StateHash {
    std::size_t operator()(const State& state) const noexcept {
        return std::hash<common::CellIndex>{}(state.cell) ^
               (std::hash<common::TimeStep>{}(state.time) << 1U);
    }
};

struct QueueNode {
    State state;
    std::uint64_t g{0};
    std::uint64_t f{0};
};

struct CompareQueueNode {
    bool operator()(const QueueNode& lhs, const QueueNode& rhs) const noexcept {
        if (lhs.f == rhs.f) {
            return lhs.g < rhs.g;
        }
        return lhs.f > rhs.f;
    }
};

std::uint64_t fallback_heuristic(common::CellIndex from, common::CellIndex to) {
    // Cell indices do not encode distance for arbitrary connectivity graphs.
    // A zero heuristic keeps this reusable low-level search optimal.
    (void)from;
    (void)to;
    return 0;
}

std::vector<common::CellIndex> reconstruct_path(
    const State& goal,
    const std::unordered_map<State, State, StateHash>& parent
) {
    std::vector<common::CellIndex> reversed;
    State current = goal;
    reversed.push_back(current.cell);
    while (true) {
        const auto it = parent.find(current);
        if (it == parent.end()) {
            break;
        }
        current = it->second;
        reversed.push_back(current.cell);
    }
    std::reverse(reversed.begin(), reversed.end());
    return reversed;
}

}  // namespace

LowLevelSearchResult SpaceTimeAStar::find_path(
    const instance::InstanceModel& instance,
    common::AgentId agent_id,
    const search::ConstraintTable& constraints,
    const search::ReservationTable& reservations,
    const LowLevelSearchOptions& options
) {
    LowLevelSearchResult result;
    const auto* task = instance.find_agent_task(agent_id);
    if (!task || !task->visit_sequence.has_minimum_points()) {
        result.status = LowLevelStatus::Error;
        result.message = "agent task is missing or invalid";
        return result;
    }

    const auto start = task->visit_sequence.start();
    const auto goal = task->visit_sequence.goal();
    const auto start_time = task->start_time;
    const auto max_time = options.max_time == 0 ? common::TimeStep{256} : options.max_time;

    if (!instance.world().is_walkable(start) || !instance.world().is_walkable(goal)) {
        result.status = LowLevelStatus::Infeasible;
        result.message = "start or goal is not walkable";
        return result;
    }

    std::priority_queue<QueueNode, std::vector<QueueNode>, CompareQueueNode> open;
    std::unordered_map<State, std::uint64_t, StateHash> best_g;
    std::unordered_map<State, State, StateHash> parent;

    const State initial{start, start_time};
    if (constraints.violates_vertex(start, start_time) ||
        reservations.reserved(start, start_time)) {
        result.status = LowLevelStatus::Infeasible;
        result.message = "start cell is constrained or reserved";
        return result;
    }
    open.push(QueueNode{initial, 0, fallback_heuristic(start, goal)});
    best_g[initial] = 0;
    result.stats.generated_nodes = 1;

    while (!open.empty()) {
        const auto current = open.top();
        open.pop();
        ++result.stats.expanded_nodes;

        if (options.node_limit > 0 && result.stats.expanded_nodes > options.node_limit) {
            result.status = LowLevelStatus::NodeLimit;
            result.message = "low-level node limit reached";
            return result;
        }

        if (current.state.cell == goal &&
            !constraints.has_future_vertex_constraint(goal, current.state.time)) {
            result.status = LowLevelStatus::Success;
            result.path.agent_id = agent_id;
            result.path.start_time = start_time;
            result.path.cells = reconstruct_path(current.state, parent);
            result.stats.generated_nodes = std::max(
                result.stats.generated_nodes,
                static_cast<std::uint64_t>(result.path.cells.size())
            );
            return result;
        }

        if (current.state.time >= max_time) {
            continue;
        }

        auto next_cells = instance.world().neighbors(current.state.cell);
        next_cells.push_back(current.state.cell);
        for (const auto next_cell : next_cells) {
            const auto next_time = static_cast<common::TimeStep>(current.state.time + 1);
            if (!instance.world().is_walkable(next_cell)) {
                continue;
            }
            if (constraints.violates_vertex(next_cell, next_time) ||
                constraints.violates_edge(current.state.cell, next_cell, next_time) ||
                reservations.reserved(next_cell, next_time) ||
                reservations.edge_reserved(current.state.cell, next_cell, next_time)) {
                continue;
            }

            const State next{next_cell, next_time};
            const auto next_g = current.g + 1;
            const auto best_it = best_g.find(next);
            if (best_it != best_g.end() && best_it->second <= next_g) {
                continue;
            }
            best_g[next] = next_g;
            parent[next] = current.state;
            const auto f = next_g + fallback_heuristic(next_cell, goal);
            open.push(QueueNode{next, next_g, f});
            ++result.stats.generated_nodes;
        }
    }

    result.status = LowLevelStatus::Infeasible;
    result.message = "no path found";
    return result;
}

}  // namespace skymapf::algorithms::low_level
