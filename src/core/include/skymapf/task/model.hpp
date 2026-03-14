/**
 * @file model.hpp
 * @brief Defines task requirement models and the task aggregate class.
 */
#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

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

    /// Returns true when the sequence has at least start and goal checkpoints.
    bool has_minimum_points() const noexcept;
    /// Returns the first checkpoint (start cell).
    common::CellIndex start() const noexcept;
    /// Returns the last checkpoint (goal cell).
    common::CellIndex goal() const noexcept;

    /**
     * @brief Serializes a sequence model to ordered JSON.
     *
     * @param j Output JSON object.
     * @param sequence_model Sequence model to serialize.
     */
    friend void to_json(nlohmann::ordered_json& j, const SequenceModel& sequence_model);

    /**
     * @brief Deserializes a sequence model from JSON.
     *
     * @param j Input JSON object.
     * @param sequence_model Output sequence model.
     */
    friend void from_json(const nlohmann::json& j, SequenceModel& sequence_model);
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

    /**
     * @brief Serializes one agent task entry to ordered JSON.
     *
     * @param j Output JSON object.
     * @param agent_task_entry Agent task entry to serialize.
     */
    friend void to_json(nlohmann::ordered_json& j, const AgentTaskEntry& agent_task_entry);

    /**
     * @brief Deserializes one agent task entry from JSON.
     *
     * @param j Input JSON object.
     * @param agent_task_entry Output agent task entry.
     */
    friend void from_json(const nlohmann::json& j, AgentTaskEntry& agent_task_entry);
};

/**
 * @brief Mutable multi-agent task aggregate with helper APIs.
 *
 * TaskModel represents "what agents need to do", not runtime progress.
 */
class TaskModel {
public:
    /// Constructs a task with auto-generated id and optional default name policy.
    TaskModel();
    /// Constructs a task with explicit id and optional display name.
    explicit TaskModel(common::TaskId task_id, std::string name = {});

    /// Returns stable task id for program references and serialization.
    const common::TaskId& id() const noexcept;
    /// Sets task id.
    void set_id(common::TaskId task_id) noexcept;
    /// Returns task id using the legacy accessor name.
    const common::TaskId& task_id() const noexcept;
    /// Sets task id using the legacy accessor name.
    void set_task_id(common::TaskId task_id) noexcept;

    /// Returns display name for logging and export.
    const std::string& name() const noexcept;
    /// Returns whether display name is non-empty.
    bool has_name() const noexcept;
    /// Sets display name.
    void set_name(std::string name);
    /// Returns additional task description text for metadata and export.
    const std::string& info() const noexcept;
    /// Returns whether additional task description text is non-empty.
    bool has_info() const noexcept;
    /// Sets additional task description text.
    void set_info(std::string info);

    /// Returns number of agent entries in this task.
    std::size_t agent_count() const noexcept;
    /// Returns whether this task has no agent entries.
    bool empty() const noexcept;

    /// Returns all agent entries in insertion order.
    const std::vector<AgentTaskEntry>& agent_entries() const noexcept;

    /// Returns whether one agent entry exists for the given agent id.
    bool has_agent(common::AgentId agent_id) const noexcept;
    /// Returns pointer to one agent entry; null when not found.
    const AgentTaskEntry* find_agent_entry(common::AgentId agent_id) const noexcept;

    /// Adds one complete agent entry; returns false when agent already exists.
    bool add_agent_entry(AgentTaskEntry agent_task_entry);

    /// Adds one agent entry from split arguments; returns false when agent already exists.
    bool add_agent_entry(
        common::AgentId agent_id,
        SequenceModel visit_sequence,
        common::TimeStep start_time = 0,
        GoalArrivalBehavior goal_behavior = GoalArrivalBehavior::StayAtGoal
    );

    /// Inserts or replaces one complete agent entry; returns true when inserted.
    bool upsert_agent_entry(AgentTaskEntry agent_task_entry);

    /// Inserts or replaces one agent entry from split arguments.
    void upsert_agent_entry(
        common::AgentId agent_id,
        SequenceModel visit_sequence,
        common::TimeStep start_time = 0,
        GoalArrivalBehavior goal_behavior = GoalArrivalBehavior::StayAtGoal
    );

    /// Removes one agent entry by agent id; returns true when removed.
    bool remove_agent_entry(common::AgentId agent_id);

    /**
     * @brief Serializes a task model to ordered JSON.
     *
     * Output fields:
     * - id
     * - name
     * - info
     * - entries
     *
     * @param j Output JSON object.
     * @param task_model Task model to serialize.
     */
    friend void to_json(nlohmann::ordered_json& j, const TaskModel& task_model);

    /**
     * @brief Deserializes a task model from JSON.
     *
     * @param j Input JSON object.
     * @param task_model Output task model.
     */
    friend void from_json(const nlohmann::json& j, TaskModel& task_model);

private:
    std::optional<std::size_t> find_index(common::AgentId agent_id) const noexcept;

    common::TaskId id_;
    std::string name_;
    std::string info_;
    std::vector<AgentTaskEntry> agent_tasks_;
};

}  // namespace skymapf::task
