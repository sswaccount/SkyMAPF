/**
 * @file model.cpp
 * @brief Implements task data model helpers and aggregate APIs.
 */


#include <algorithm>
#include <iterator>
#include <utility>

#include "skymapf/task/model.hpp"
#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/id_generator.hpp"

namespace skymapf::task {

bool SequenceModel::has_minimum_points() const noexcept {
    return checkpoints.size() >= 2;
}

common::CellIndex SequenceModel::start() const noexcept {
    return checkpoints.front();
}

common::CellIndex SequenceModel::goal() const noexcept {
    return checkpoints.back();
}

TaskModel::TaskModel()
    : TaskModel(utils::IdGenerator::next_task_id(), {}) {}

TaskModel::TaskModel(common::TaskId task_id, std::string name) {
    id_ = task_id;
    name_ = name.empty() ? utils::DefaultNaming::task_name(0, task_id) : std::move(name);
}

TaskModel TaskModel::create(common::TaskId task_id, std::string name) {
    return TaskModel(task_id, std::move(name));
}

const common::TaskId& TaskModel::id() const noexcept {
    return id_;
}

void TaskModel::set_id(common::TaskId task_id) noexcept {
    id_ = task_id;
}

common::TaskId TaskModel::task_id() const noexcept {
    return id();
}

void TaskModel::set_task_id(common::TaskId task_id) noexcept {
    set_id(task_id);
}

const std::string& TaskModel::name() const noexcept {
    return name_;
}

bool TaskModel::has_name() const noexcept {
    return !name_.empty();
}

void TaskModel::set_name(std::string name) {
    name_ = std::move(name);
}

bool TaskModel::is_available(common::TimeStep now) const noexcept {
    const auto earliest = earliest_start_time();
    if (!earliest.has_value()) {
        return false;
    }
    return now >= *earliest;
}

std::size_t TaskModel::agent_count() const noexcept {
    return agent_tasks.size();
}

bool TaskModel::empty() const noexcept {
    return agent_tasks.empty();
}

const std::vector<SingleAgentTaskModel>& TaskModel::agent_sequences() const noexcept {
    return agent_tasks;
}

bool TaskModel::has_agent(common::AgentId agent_id) const noexcept {
    return find_index(agent_id).has_value();
}

const SingleAgentTaskModel* TaskModel::find_agent_sequence(common::AgentId agent_id) const noexcept {
    const auto idx = find_index(agent_id);
    if (!idx.has_value()) {
        return nullptr;
    }
    return &agent_tasks[*idx];
}

bool TaskModel::add_agent_sequence(
    common::AgentId agent_id,
    SequenceModel visit_sequence,
    common::TimeStep start_time,
    GoalArrivalBehavior goal_behavior
) {
    if (has_agent(agent_id)) {
        return false;
    }
    agent_tasks.push_back(SingleAgentTaskModel{
        agent_id,
        start_time,
        std::move(visit_sequence),
        goal_behavior
    });
    return true;
}

void TaskModel::upsert_agent_sequence(
    common::AgentId agent_id,
    SequenceModel visit_sequence,
    common::TimeStep start_time,
    GoalArrivalBehavior goal_behavior
) {
    const auto idx = find_index(agent_id);
    if (idx.has_value()) {
        agent_tasks[*idx].start_time = start_time;
        agent_tasks[*idx].visit_sequence = std::move(visit_sequence);
        agent_tasks[*idx].goal_behavior = goal_behavior;
        return;
    }
    agent_tasks.push_back(SingleAgentTaskModel{
        agent_id,
        start_time,
        std::move(visit_sequence),
        goal_behavior
    });
}

std::optional<common::TimeStep> TaskModel::earliest_start_time() const noexcept {
    if (agent_tasks.empty()) {
        return std::nullopt;
    }
    common::TimeStep earliest = agent_tasks.front().start_time;
    for (const auto& seq : agent_tasks) {
        if (seq.start_time < earliest) {
            earliest = seq.start_time;
        }
    }
    return earliest;
}

bool TaskModel::remove_agent_sequence(common::AgentId agent_id) {
    const auto idx = find_index(agent_id);
    if (!idx.has_value()) {
        return false;
    }
    agent_tasks.erase(agent_tasks.begin() + static_cast<std::ptrdiff_t>(*idx));
    return true;
}

std::optional<std::size_t> TaskModel::find_index(common::AgentId agent_id) const noexcept {
    const auto it = std::find_if(
        agent_tasks.begin(),
        agent_tasks.end(),
        [agent_id](const SingleAgentTaskModel& seq) { return seq.agent_id == agent_id; }
    );
    if (it == agent_tasks.end()) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(std::distance(agent_tasks.begin(), it));
}

}  // namespace skymapf::task
