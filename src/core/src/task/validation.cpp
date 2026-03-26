/**
 * @file validation.cpp
 * @brief Implements semantic validation for generated and loaded tasks.
 */
#include "skymapf/task/validation.hpp"

#include <iterator>
#include <unordered_set>
#include <utility>

#include "skymapf/generator/task_generator.hpp"
#include "skymapf/world/model.hpp"

namespace skymapf::task {

namespace {

void add_issue(
    TaskValidationResult& result,
    TaskValidationCode code,
    common::TaskId task_id,
    std::string message,
    std::optional<std::size_t> agent_index = std::nullopt
) {
    result.ok = false;
    result.issues.push_back(TaskValidationIssue{code, task_id, std::move(message), agent_index});
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
    const std::vector<agent::AgentModel>& agents,
    const world::WorldModel& world,
    bool enforce_one_active_task_per_agent
) {
    TaskValidationResult aggregate;
    std::unordered_set<common::AgentId> known_agents;
    known_agents.reserve(agents.size());
    for (const auto& agent : agents) {
        known_agents.insert(agent.id());
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

TaskValidationResult validate_generated_task(
    const TaskModel& task,
    const world::WorldModel& world,
    const generator::TaskGenerationOptions& options
) {
    TaskValidationResult result;

    if (task.agent_count() != options.agent_count) {
        add_issue(
            result,
            TaskValidationCode::GeneratedAgentCountMismatch,
            task.task_id(),
            "Generated agent count does not match requested agent_count."
        );
    }

    const auto& assignments = task.agent_sequences();
    for (std::size_t i = 0; i < assignments.size(); ++i) {
        const auto& assignment = assignments[i];
        if (assignment.start_time > options.max_start_time) {
            add_issue(
                result,
                TaskValidationCode::GeneratedInvalidStartTimeRange,
                task.task_id(),
                "Agent start_time is outside [0, max_start_time].",
                i
            );
        }

        const auto seq_size = assignment.visit_sequence.checkpoints.size();
        if (seq_size < 2) {
            add_issue(
                result,
                TaskValidationCode::NotEnoughCheckpoints,
                task.task_id(),
                "Visit sequence length is smaller than domain minimum (2).",
                i
            );
            continue;
        }
        if (seq_size > options.max_sequence_size) {
            add_issue(
                result,
                TaskValidationCode::GeneratedSequenceTooLong,
                task.task_id(),
                "Visit sequence length exceeds max_sequence_size.",
                i
            );
        }

        const auto start_cell = assignment.visit_sequence.start();
        if (!world.is_valid_index(start_cell) || !world.is_walkable(start_cell)) {
            add_issue(
                result,
                TaskValidationCode::GeneratedInvalidStartCell,
                task.task_id(),
                "Visit sequence start cell is invalid or non-walkable.",
                i
            );
        }

        for (const auto checkpoint : assignment.visit_sequence.checkpoints) {
            if (!world.is_valid_index(checkpoint)) {
                add_issue(
                    result,
                    TaskValidationCode::InvalidCellIndex,
                    task.task_id(),
                    "Visit sequence contains checkpoint outside world bounds.",
                    i
                );
                continue;
            }
            if (!world.is_walkable(checkpoint)) {
                add_issue(
                    result,
                    TaskValidationCode::NonWalkableCheckpoint,
                    task.task_id(),
                    "Visit sequence contains non-walkable checkpoint.",
                    i
                );
            }
        }
    }

    return result;
}

}  // namespace skymapf::task
