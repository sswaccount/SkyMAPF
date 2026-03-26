/**
 * @file model.hpp
 * @brief Defines task requirement models and the task aggregate class.
 */
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "skymapf/common/ids.hpp"
#include "skymapf/task/policy.hpp"

namespace skymapf::task {

/**
 * @brief Defines one ordered route sequence.
 *
 * The sequence uses `[start, waypoint..., goal]` semantics.
 * At least two checkpoints are required for a valid route.
 */
struct SequenceModel {
    std::vector<common::CellIndex> checkpoints;

    bool has_minimum_points() const noexcept;
    common::CellIndex start() const noexcept;
    common::CellIndex goal() const noexcept;
};

/**
 * @brief Defines one per-agent task requirement entry.
 *
 * One entry binds one agent id to requirement metadata:
 * - when to start
 * - what route checkpoints to visit
 * - what occupancy behavior to apply after reaching the goal
 */
struct AgentTaskEntry {
    common::AgentId agent_id{0};
    common::TimeStep start_time{0};
    SequenceModel visit_sequence;
    GoalArrivalBehavior goal_behavior{GoalArrivalBehavior::StayAtGoal};
};

/// Backward-compatible alias kept during migration.
using SingleAgentTaskModel = AgentTaskEntry;

/**
 * @brief Mutable multi-agent task aggregate with helper APIs.
 *
 * TaskModel represents "what agents need to do", not runtime progress.
 */
class TaskModel {
public:
    TaskModel();
    explicit TaskModel(common::TaskId task_id, std::string name = {});

    static TaskModel create(common::TaskId task_id, std::string name = {});

    const common::TaskId& id() const noexcept;
    void set_id(common::TaskId task_id) noexcept;

    common::TaskId task_id() const noexcept;
    void set_task_id(common::TaskId task_id) noexcept;

    const std::string& name() const noexcept;
    bool has_name() const noexcept;
    void set_name(std::string name);

    bool is_available(common::TimeStep now) const noexcept;
    std::size_t agent_count() const noexcept;
    bool empty() const noexcept;

    const std::vector<AgentTaskEntry>& agent_entries() const noexcept;
    const std::vector<AgentTaskEntry>& agent_sequences() const noexcept;

    bool has_agent(common::AgentId agent_id) const noexcept;
    const AgentTaskEntry* find_agent_entry(common::AgentId agent_id) const noexcept;
    const AgentTaskEntry* find_agent_sequence(common::AgentId agent_id) const noexcept;

    bool add_agent_entry(
        common::AgentId agent_id,
        SequenceModel visit_sequence,
        common::TimeStep start_time = 0,
        GoalArrivalBehavior goal_behavior = GoalArrivalBehavior::StayAtGoal
    );
    bool add_agent_sequence(
        common::AgentId agent_id,
        SequenceModel visit_sequence,
        common::TimeStep start_time = 0,
        GoalArrivalBehavior goal_behavior = GoalArrivalBehavior::StayAtGoal
    );

    void upsert_agent_entry(
        common::AgentId agent_id,
        SequenceModel visit_sequence,
        common::TimeStep start_time = 0,
        GoalArrivalBehavior goal_behavior = GoalArrivalBehavior::StayAtGoal
    );
    void upsert_agent_sequence(
        common::AgentId agent_id,
        SequenceModel visit_sequence,
        common::TimeStep start_time = 0,
        GoalArrivalBehavior goal_behavior = GoalArrivalBehavior::StayAtGoal
    );

    std::optional<common::TimeStep> earliest_start_time() const noexcept;
    bool remove_agent_entry(common::AgentId agent_id);
    bool remove_agent_sequence(common::AgentId agent_id);

private:
    std::optional<std::size_t> find_index(common::AgentId agent_id) const noexcept;

    common::TaskId id_;
    std::string name_;
    std::vector<AgentTaskEntry> agent_tasks_;
};

}  // namespace skymapf::task
