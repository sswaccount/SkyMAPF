/**
 * @file model.cpp
 * @brief Implements task data model helpers and aggregate APIs.
 */


#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <utility>

#include "skymapf/task/model.hpp"
#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/random_tool.hpp"

namespace skymapf::task {

namespace {

const char* goal_behavior_to_string(GoalArrivalBehavior behavior) noexcept {
    switch (behavior) {
    case GoalArrivalBehavior::DisappearAtGoal:
        return "disappear_at_goal";
    case GoalArrivalBehavior::StayAtGoal:
        return "stay_at_goal";
    default:
        return "stay_at_goal";
    }
}

GoalArrivalBehavior goal_behavior_from_string(const std::string& text) {
    if (text == "disappear_at_goal") {
        return GoalArrivalBehavior::DisappearAtGoal;
    }
    if (text == "stay_at_goal") {
        return GoalArrivalBehavior::StayAtGoal;
    }
    throw std::runtime_error("AgentTaskEntry field 'goal_behavior' has unknown value.");
}

}  // namespace

bool SequenceModel::has_minimum_points() const noexcept {
    return checkpoints.size() >= 2;
}

common::CellIndex SequenceModel::start() const noexcept {
    return checkpoints.front();
}

common::CellIndex SequenceModel::goal() const noexcept {
    return checkpoints.back();
}

void to_json(nlohmann::ordered_json& j, const SequenceModel& sequence_model) {
    j = nlohmann::ordered_json{
        {"checkpoints", sequence_model.checkpoints}
    };
}

void from_json(const nlohmann::json& j, SequenceModel& sequence_model) {
    if (!j.contains("checkpoints")) {
        throw std::runtime_error("SequenceModel JSON missing 'checkpoints'.");
    }
    sequence_model.checkpoints = j.at("checkpoints").get<std::vector<common::CellIndex>>();
}

TaskModel::TaskModel()
    : TaskModel(
        utils::RandomTool::instance().next_id_value(), 
        {}
    ) {}

TaskModel::TaskModel(common::TaskId task_id, std::string name) {
    id_ = task_id;
    name_ = std::move(name);
    if (name_.empty()) {
        name_ = utils::DefaultNaming::next_task_name(0);
    }
}

const common::TaskId& TaskModel::id() const noexcept {
    return id_;
}

void TaskModel::set_id(common::TaskId task_id) noexcept {
    id_ = task_id;
}

const common::TaskId& TaskModel::task_id() const noexcept {
    return id_;
}

void TaskModel::set_task_id(common::TaskId task_id) noexcept {
    id_ = task_id;
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

const std::string& TaskModel::info() const noexcept {
    return info_;
}

bool TaskModel::has_info() const noexcept {
    return !info_.empty();
}

void TaskModel::set_info(std::string info) {
    info_ = std::move(info);
}

std::size_t TaskModel::agent_count() const noexcept {
    return agent_tasks_.size();
}

bool TaskModel::empty() const noexcept {
    return agent_tasks_.empty();
}

const std::vector<AgentTaskEntry>& TaskModel::agent_entries() const noexcept {
    return agent_tasks_;
}

bool TaskModel::has_agent(common::AgentId agent_id) const noexcept {
    return find_index(agent_id).has_value();
}

const AgentTaskEntry* TaskModel::find_agent_entry(common::AgentId agent_id) const noexcept {
    const auto idx = find_index(agent_id);
    if (!idx.has_value()) {
        return nullptr;
    }
    return &agent_tasks_[*idx];
}

bool TaskModel::add_agent_entry(AgentTaskEntry agent_task_entry) {
    return add_agent_entry(
        agent_task_entry.agent_id,
        std::move(agent_task_entry.visit_sequence),
        agent_task_entry.start_time,
        agent_task_entry.goal_behavior
    );
}

bool TaskModel::add_agent_entry(
    common::AgentId agent_id,
    SequenceModel visit_sequence,
    common::TimeStep start_time,
    GoalArrivalBehavior goal_behavior
) {
    if (has_agent(agent_id)) {
        return false;
    }
    agent_tasks_.push_back(AgentTaskEntry{
        agent_id,
        start_time,
        std::move(visit_sequence),
        goal_behavior
    });
    return true;
}

bool TaskModel::upsert_agent_entry(AgentTaskEntry agent_task_entry) {
    const auto idx = find_index(agent_task_entry.agent_id);
    if (idx.has_value()) {
        agent_tasks_[*idx] = std::move(agent_task_entry);
        return false;
    }
    agent_tasks_.push_back(std::move(agent_task_entry));
    return true;
}

void TaskModel::upsert_agent_entry(
    common::AgentId agent_id,
    SequenceModel visit_sequence,
    common::TimeStep start_time,
    GoalArrivalBehavior goal_behavior
) {
    (void)upsert_agent_entry(AgentTaskEntry{
        agent_id,
        start_time,
        std::move(visit_sequence),
        goal_behavior
    });
}

bool TaskModel::remove_agent_entry(common::AgentId agent_id) {
    const auto idx = find_index(agent_id);
    if (!idx.has_value()) {
        return false;
    }
    agent_tasks_.erase(agent_tasks_.begin() + static_cast<std::ptrdiff_t>(*idx));
    return true;
}

std::optional<std::size_t> TaskModel::find_index(common::AgentId agent_id) const noexcept {
    const auto it = std::find_if(
        agent_tasks_.begin(),
        agent_tasks_.end(),
        [agent_id](const AgentTaskEntry& seq) { return seq.agent_id == agent_id; }
    );
    if (it == agent_tasks_.end()) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(std::distance(agent_tasks_.begin(), it));
}

void to_json(nlohmann::ordered_json& j, const AgentTaskEntry& agent_task_entry) {
    j = nlohmann::ordered_json{
        {"agent_id", agent_task_entry.agent_id},
        {"start_time", agent_task_entry.start_time},
        {"visit_sequence", agent_task_entry.visit_sequence},
        {"goal_behavior", goal_behavior_to_string(agent_task_entry.goal_behavior)}
    };
}

void from_json(const nlohmann::json& j, AgentTaskEntry& agent_task_entry) {
    if (!j.contains("agent_id")) {
        throw std::runtime_error("AgentTaskEntry JSON missing 'agent_id'.");
    }
    if (!j.contains("start_time")) {
        throw std::runtime_error("AgentTaskEntry JSON missing 'start_time'.");
    }
    if (!j.contains("visit_sequence")) {
        throw std::runtime_error("AgentTaskEntry JSON missing 'visit_sequence'.");
    }
    if (!j.contains("goal_behavior")) {
        throw std::runtime_error("AgentTaskEntry JSON missing 'goal_behavior'.");
    }

    agent_task_entry.agent_id = j.at("agent_id").get<common::AgentId>();
    agent_task_entry.start_time = j.at("start_time").get<common::TimeStep>();
    agent_task_entry.visit_sequence = j.at("visit_sequence").get<SequenceModel>();
    agent_task_entry.goal_behavior =
        goal_behavior_from_string(j.at("goal_behavior").get<std::string>());
}

void to_json(nlohmann::ordered_json& j, const TaskModel& task_model) {
    j = nlohmann::ordered_json{
        {"id", task_model.id_},
        {"name", task_model.name_},
        {"info", task_model.info_},
        {"entries", task_model.agent_tasks_}
    };
}

void from_json(const nlohmann::json& j, TaskModel& task_model) {
    if (!j.contains("id")) {
        throw std::runtime_error("TaskModel JSON missing 'id'.");
    }
    if (!j.contains("entries")) {
        throw std::runtime_error("TaskModel JSON missing 'entries'.");
    }

    TaskModel parsed(
        j.at("id").get<common::TaskId>(),
        j.value("name", std::string{})
    );
    parsed.info_ = j.value("info", std::string{});
    parsed.agent_tasks_ = j.at("entries").get<std::vector<AgentTaskEntry>>();
    task_model = std::move(parsed);
}

}  // namespace skymapf::task
