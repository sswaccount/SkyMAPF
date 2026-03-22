/**
 * @file validation.hpp
 * @brief Declares task validation routines and issue categories.
 */
#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../agent/agent.hpp"
#include "model.hpp"

namespace skymapf::world {
class WorldModel;
}
namespace skymapf::generator {
struct TaskGenerationOptions;
}

namespace skymapf::task {

/// Categorizes task validation failures.
enum class TaskValidationCode {
    EmptyTaskAgentSet,
    EmptyCheckpointSequence,
    NotEnoughCheckpoints,
    InvalidCellIndex,
    NonWalkableCheckpoint,
    DuplicateAgentInTask,
    DuplicateAgentTask,
    AgentNotFound,
    GeneratedAgentCountMismatch,
    GeneratedInvalidStartTimeRange,
    GeneratedSequenceTooLong,
    GeneratedInvalidStartCell,
};

/// Describes one validation issue with context.
struct TaskValidationIssue {
    TaskValidationCode code{TaskValidationCode::NotEnoughCheckpoints};
    common::TaskId task_id{0};
    std::string message;
    std::optional<std::size_t> agent_index;
};

/// Aggregates validation status and issue list.
struct TaskValidationResult {
    bool ok{true};
    std::vector<TaskValidationIssue> issues;
};

/**
 * @brief Validates one task against world constraints.
 *
 * @param task Task to validate.
 * @param world World providing index bounds and walkability checks.
 * @return Validation result with issues when validation fails.
 */
TaskValidationResult validate_task(const TaskModel& task, const world::WorldModel& world);

/**
 * @brief Validates a task set with optional cross-task constraints.
 *
 * @param tasks Tasks to validate.
 * @param agents Known agents referenced by tasks.
 * @param world World used for spatial checks.
 * @param enforce_one_active_task_per_agent Enables duplicate assignment checks.
 * @return Aggregated validation result across all tasks.
 */
TaskValidationResult validate_tasks(
    const std::vector<TaskModel>& tasks,
    const std::vector<agent::AgentSpec>& agents,
    const world::WorldModel& world,
    bool enforce_one_active_task_per_agent = true
);

/**
 * @brief Validates one generated task against generation options and world constraints.
 *
 * @param task Generated task model.
 * @param world World used for spatial checks.
 * @param options Options used for generation.
 * @return Validation result for generation-time invariants.
 */
TaskValidationResult validate_generated_task(
    const TaskModel& task,
    const world::WorldModel& world,
    const generator::TaskGenerationOptions& options
);

}  // namespace skymapf::task
