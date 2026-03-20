/**
 * @file task.hpp
 * @brief Defines task data structures with per-agent visit sequences.
 */
#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include "skymapf/common/ids.hpp"

namespace skymapf::task {

/// Represents lifecycle states of one agent-specific task sequence.
enum class AgentTaskStatus {
    Pending,
    Active,
    Completed,
    Failed,
};

/**
 * @brief Ordered checkpoint sequence for one route.
 *
 * The sequence follows [start, waypoint..., goal] semantics.
 */
struct VisitSequence {
    // Ordered sequence: [start, waypoint..., goal].
    std::vector<common::CellIndex> checkpoints;

    bool has_minimum_points() const noexcept {
        return checkpoints.size() >= 2;
    }

    common::CellIndex start() const noexcept {
        return checkpoints.front();
    }

    common::CellIndex goal() const noexcept {
        return checkpoints.back();
    }
};

/// Binds one agent to its sequence, start time, and lifecycle state.
struct AgentVisitSequence {
    common::AgentId agent_id{0};
    common::TimeStep start_time{0};
    VisitSequence visit_sequence;
    AgentTaskStatus status{AgentTaskStatus::Pending};
};

/**
 * @brief Mutable multi-agent task definition.
 *
 * A task contains one or more agent sequences. Each sequence carries
 * its own start time and lifecycle status.
 */
class Task {
public:
    Task() = default;

    explicit Task(common::TaskId task_id, std::string name = {})
        : task_id_(task_id), name_(std::move(name)) {}

    /// Creates a task object with optional display name.
    static Task create(common::TaskId task_id, std::string name = {}) {
        return Task(task_id, std::move(name));
    }

    common::TaskId task_id() const noexcept { return task_id_; }
    void set_task_id(common::TaskId task_id) noexcept { task_id_ = task_id; }

    const std::string& name() const noexcept { return name_; }
    void set_name(std::string name) { name_ = std::move(name); }

    /// Returns whether the task is available at time @p now.
    bool is_available(common::TimeStep now) const noexcept {
        if (agent_sequences_.empty()) {
            return false;
        }
        common::TimeStep earliest = agent_sequences_.front().start_time;
        for (const auto& seq : agent_sequences_) {
            if (seq.start_time < earliest) {
                earliest = seq.start_time;
            }
        }
        return now >= earliest;
    }

    /// Returns number of agent sequences in this task.
    std::size_t agent_count() const noexcept {
        return agent_sequences_.size();
    }

    /// Returns true when the task has no agent sequence.
    bool empty() const noexcept {
        return agent_sequences_.empty();
    }

    /// Returns read-only view of all agent sequences.
    const std::vector<AgentVisitSequence>& agent_sequences() const noexcept {
        return agent_sequences_;
    }

    /// Returns true when the task already contains the given agent.
    bool has_agent(common::AgentId agent_id) const noexcept {
        return find_index(agent_id).has_value();
    }

    /**
     * @brief Returns sequence pointer for a specific agent.
     *
     * @return Pointer to the sequence or nullptr when absent.
     */
    const AgentVisitSequence* find_agent_sequence(common::AgentId agent_id) const noexcept {
        const auto idx = find_index(agent_id);
        if (!idx.has_value()) {
            return nullptr;
        }
        return &agent_sequences_[*idx];
    }

    /**
     * @brief Adds a new agent sequence when the agent is not present.
     *
     * @return True when insertion succeeds; false on duplicate agent id.
     */
    bool add_agent_sequence(
        common::AgentId agent_id,
        VisitSequence visit_sequence,
        common::TimeStep start_time = 0,
        AgentTaskStatus status = AgentTaskStatus::Pending
    ) {
        if (has_agent(agent_id)) {
            return false;
        }
        agent_sequences_.push_back(AgentVisitSequence{
            agent_id,
            start_time,
            std::move(visit_sequence),
            status
        });
        return true;
    }

    /// Inserts or replaces one agent sequence by agent id.
    void upsert_agent_sequence(
        common::AgentId agent_id,
        VisitSequence visit_sequence,
        common::TimeStep start_time = 0,
        AgentTaskStatus status = AgentTaskStatus::Pending
    ) {
        const auto idx = find_index(agent_id);
        if (idx.has_value()) {
            agent_sequences_[*idx].start_time = start_time;
            agent_sequences_[*idx].visit_sequence = std::move(visit_sequence);
            agent_sequences_[*idx].status = status;
            return;
        }
        agent_sequences_.push_back(AgentVisitSequence{
            agent_id,
            start_time,
            std::move(visit_sequence),
            status
        });
    }

    /// Returns the earliest start time among all agent sequences.
    std::optional<common::TimeStep> earliest_start_time() const noexcept {
        if (agent_sequences_.empty()) {
            return std::nullopt;
        }
        common::TimeStep earliest = agent_sequences_.front().start_time;
        for (const auto& seq : agent_sequences_) {
            if (seq.start_time < earliest) {
                earliest = seq.start_time;
            }
        }
        return earliest;
    }

    /**
     * @brief Removes an agent sequence by agent id.
     *
     * @return True when a sequence was removed.
     */
    bool remove_agent_sequence(common::AgentId agent_id) {
        const auto idx = find_index(agent_id);
        if (!idx.has_value()) {
            return false;
        }
        agent_sequences_.erase(agent_sequences_.begin() + static_cast<std::ptrdiff_t>(*idx));
        return true;
    }

private:
    std::optional<std::size_t> find_index(common::AgentId agent_id) const noexcept {
        const auto it = std::find_if(
            agent_sequences_.begin(),
            agent_sequences_.end(),
            [agent_id](const AgentVisitSequence& seq) { return seq.agent_id == agent_id; }
        );
        if (it == agent_sequences_.end()) {
            return std::nullopt;
        }
        return static_cast<std::size_t>(std::distance(agent_sequences_.begin(), it));
    }

    common::TaskId task_id_{0};
    std::string name_;
    std::vector<AgentVisitSequence> agent_sequences_;
};

}  // namespace skymapf::task
