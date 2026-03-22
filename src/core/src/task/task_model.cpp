/**
 * @file task_model.cpp
 * @brief Implements task data model helpers and aggregate APIs.
 */
#include "skymapf/task/task_model.hpp"

#include <algorithm>
#include <iterator>
#include <utility>

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

TaskModel::TaskModel(common::TaskId task_id, std::string name) {
    data_.task_id = task_id;
    data_.name = std::move(name);
}

TaskModel TaskModel::create(common::TaskId task_id, std::string name) {
    return TaskModel(task_id, std::move(name));
}

common::TaskId TaskModel::task_id() const noexcept {
    return data_.task_id;
}

void TaskModel::set_task_id(common::TaskId task_id) noexcept {
    data_.task_id = task_id;
}

const std::string& TaskModel::name() const noexcept {
    return data_.name;
}

void TaskModel::set_name(std::string name) {
    data_.name = std::move(name);
}

bool TaskModel::is_available(common::TimeStep now) const noexcept {
    const auto earliest = earliest_start_time();
    if (!earliest.has_value()) {
        return false;
    }
    return now >= *earliest;
}

std::size_t TaskModel::agent_count() const noexcept {
    return data_.agent_sequences.size();
}

bool TaskModel::empty() const noexcept {
    return data_.agent_sequences.empty();
}

const std::vector<SingleAgentTaskModel>& TaskModel::agent_sequences() const noexcept {
    return data_.agent_sequences;
}

bool TaskModel::has_agent(common::AgentId agent_id) const noexcept {
    return find_index(agent_id).has_value();
}

const SingleAgentTaskModel* TaskModel::find_agent_sequence(common::AgentId agent_id) const noexcept {
    const auto idx = find_index(agent_id);
    if (!idx.has_value()) {
        return nullptr;
    }
    return &data_.agent_sequences[*idx];
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
    data_.agent_sequences.push_back(SingleAgentTaskModel{
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
        data_.agent_sequences[*idx].start_time = start_time;
        data_.agent_sequences[*idx].visit_sequence = std::move(visit_sequence);
        data_.agent_sequences[*idx].goal_behavior = goal_behavior;
        return;
    }
    data_.agent_sequences.push_back(SingleAgentTaskModel{
        agent_id,
        start_time,
        std::move(visit_sequence),
        goal_behavior
    });
}

std::optional<common::TimeStep> TaskModel::earliest_start_time() const noexcept {
    if (data_.agent_sequences.empty()) {
        return std::nullopt;
    }
    common::TimeStep earliest = data_.agent_sequences.front().start_time;
    for (const auto& seq : data_.agent_sequences) {
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
    data_.agent_sequences.erase(data_.agent_sequences.begin() + static_cast<std::ptrdiff_t>(*idx));
    return true;
}

std::optional<std::size_t> TaskModel::find_index(common::AgentId agent_id) const noexcept {
    const auto it = std::find_if(
        data_.agent_sequences.begin(),
        data_.agent_sequences.end(),
        [agent_id](const SingleAgentTaskModel& seq) { return seq.agent_id == agent_id; }
    );
    if (it == data_.agent_sequences.end()) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(std::distance(data_.agent_sequences.begin(), it));
}

}  // namespace skymapf::task
