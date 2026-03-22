/**
 * @file validation.cpp
 * @brief Implements semantic validation for generated and loaded tasks.
 */
#include "skymapf/task/validation.hpp"

#include <iterator>
#include <unordered_set>
#include <utility>

#include "skymapf/world/world_model.hpp"

namespace skymapf::task {

namespace {

void add_issue(
    TaskValidationResult& result,
    TaskValidationCode code,
    common::TaskId task_id,
    std::string message
) {
    result.ok = false;
    result.issues.push_back(TaskValidationIssue{code, task_id, std::move(message)});
}

}  // namespace

TaskValidationResult validate_task(const TaskModel& task, const world::WorldModel& world) {
    TaskValidationResult result;

    if (task.empty()) {
        add_issue(
            result,
            TaskValidationCode::EmptyTaskAgentSet,
            task.task_id(),
            "Task does not contain any agent sequence."
        );
        return result;
    }

    std::unordered_set<common::AgentId> seen_agents;
    seen_agents.reserve(task.agent_count());
    for (const auto& seq : task.agent_sequences()) {
        const auto [_, inserted] = seen_agents.insert(seq.agent_id);
        if (!inserted) {
            add_issue(
                result,
                TaskValidationCode::DuplicateAgentInTask,
                task.task_id(),
                "Task contains duplicate sequence entries for one agent."
            );
        }

        const auto& checkpoints = seq.visit_sequence.checkpoints;
        if (checkpoints.empty()) {
            add_issue(
                result,
                TaskValidationCode::EmptyCheckpointSequence,
                task.task_id(),
                "Agent sequence checkpoint list is empty."
            );
            continue;
        }
        if (checkpoints.size() < 2) {
            add_issue(
                result,
                TaskValidationCode::NotEnoughCheckpoints,
                task.task_id(),
                "Each agent sequence requires at least start and goal checkpoints."
            );
        }

        for (const auto checkpoint : checkpoints) {
            if (!world.is_valid_index(checkpoint)) {
                add_issue(
                    result,
                    TaskValidationCode::InvalidCellIndex,
                    task.task_id(),
                    "Checkpoint index is outside world bounds."
                );
                continue;
            }
            if (!world.is_walkable(checkpoint)) {
                add_issue(
                    result,
                    TaskValidationCode::NonWalkableCheckpoint,
                    task.task_id(),
                    "Checkpoint is on a non-walkable cell."
                );
            }
        }
    }

    return result;
}

TaskValidationResult validate_tasks(
    const std::vector<TaskModel>& tasks,
    const std::vector<agent::AgentSpec>& agents,
    const world::WorldModel& world,
    bool enforce_one_active_task_per_agent
) {
    TaskValidationResult aggregate;
    std::unordered_set<common::AgentId> known_agents;
    known_agents.reserve(agents.size());
    for (const auto& agent : agents) {
        known_agents.insert(agent.agent_id);
    }

    std::unordered_set<common::AgentId> assigned_agents;
    assigned_agents.reserve(tasks.size());

    for (const auto& task : tasks) {
        for (const auto& seq : task.agent_sequences()) {
            if (known_agents.find(seq.agent_id) == known_agents.end()) {
                add_issue(
                    aggregate,
                    TaskValidationCode::AgentNotFound,
                    task.task_id(),
                    "Task references an unknown agent_id."
                );
            }

            if (enforce_one_active_task_per_agent) {
                const auto [_, inserted] = assigned_agents.insert(seq.agent_id);
                if (!inserted) {
                    add_issue(
                        aggregate,
                        TaskValidationCode::DuplicateAgentTask,
                        task.task_id(),
                        "Multiple tasks are assigned to the same agent."
                    );
                }
            }
        }

        auto single = validate_task(task, world);
        if (!single.ok) {
            aggregate.ok = false;
            aggregate.issues.insert(
                aggregate.issues.end(),
                std::make_move_iterator(single.issues.begin()),
                std::make_move_iterator(single.issues.end())
            );
        }
    }

    return aggregate;
}

}  // namespace skymapf::task
