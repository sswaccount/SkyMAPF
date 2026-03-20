#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include "../common/ids.hpp"

namespace skymapf::task {

enum class TaskStatus {
    Pending,
    Active,
    Completed,
    Failed,
};

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

struct TaskTiming {
    // Task becomes schedulable when now >= release_time.
    common::TimeStep release_time{0};
    // Optional hard deadline for finishing the task.
    std::optional<common::TimeStep> deadline;
};

struct AgentVisitSequence {
    common::AgentId agent_id{0};
    VisitSequence visit_sequence;
};

class Task {
public:
    Task() = default;

    explicit Task(common::TaskId task_id, TaskTiming timing = {}, std::string name = {})
        : task_id_(task_id), timing_(timing), name_(std::move(name)) {}

    static Task create(common::TaskId task_id, TaskTiming timing = {}, std::string name = {}) {
        return Task(task_id, timing, std::move(name));
    }

    common::TaskId task_id() const noexcept { return task_id_; }
    void set_task_id(common::TaskId task_id) noexcept { task_id_ = task_id; }

    const std::string& name() const noexcept { return name_; }
    void set_name(std::string name) { name_ = std::move(name); }

    const TaskTiming& timing() const noexcept { return timing_; }
    void set_timing(TaskTiming timing) noexcept { timing_ = timing; }

    TaskStatus status() const noexcept { return status_; }
    void set_status(TaskStatus status) noexcept { status_ = status; }

    bool is_available(common::TimeStep now) const noexcept {
        return now >= timing_.release_time;
    }

    std::size_t agent_count() const noexcept {
        return agent_sequences_.size();
    }

    bool empty() const noexcept {
        return agent_sequences_.empty();
    }

    const std::vector<AgentVisitSequence>& agent_sequences() const noexcept {
        return agent_sequences_;
    }

    bool has_agent(common::AgentId agent_id) const noexcept {
        return find_index(agent_id).has_value();
    }

    const AgentVisitSequence* find_agent_sequence(common::AgentId agent_id) const noexcept {
        const auto idx = find_index(agent_id);
        if (!idx.has_value()) {
            return nullptr;
        }
        return &agent_sequences_[*idx];
    }

    bool add_agent_sequence(common::AgentId agent_id, VisitSequence visit_sequence) {
        if (has_agent(agent_id)) {
            return false;
        }
        agent_sequences_.push_back(AgentVisitSequence{agent_id, std::move(visit_sequence)});
        return true;
    }

    void upsert_agent_sequence(common::AgentId agent_id, VisitSequence visit_sequence) {
        const auto idx = find_index(agent_id);
        if (idx.has_value()) {
            agent_sequences_[*idx].visit_sequence = std::move(visit_sequence);
            return;
        }
        agent_sequences_.push_back(AgentVisitSequence{agent_id, std::move(visit_sequence)});
    }

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
    TaskTiming timing_{};
    TaskStatus status_{TaskStatus::Pending};
    std::string name_;
    std::vector<AgentVisitSequence> agent_sequences_;
};

}  // namespace skymapf::task
