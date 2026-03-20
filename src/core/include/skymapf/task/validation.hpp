#pragma once

#include <string>
#include <vector>

#include "../agent/agent.hpp"
#include "task.hpp"

namespace skymapf::world {
class WorldModel;
}

namespace skymapf::task {

enum class TaskValidationCode {
    EmptyTaskAgentSet,
    EmptyCheckpointSequence,
    NotEnoughCheckpoints,
    InvalidCellIndex,
    NonWalkableCheckpoint,
    DuplicateAgentInTask,
    DuplicateAgentTask,
    AgentNotFound,
    DeadlineBeforeRelease,
};

struct TaskValidationIssue {
    TaskValidationCode code{TaskValidationCode::NotEnoughCheckpoints};
    common::TaskId task_id{0};
    std::string message;
};

struct TaskValidationResult {
    bool ok{true};
    std::vector<TaskValidationIssue> issues;
};

TaskValidationResult validate_task(const Task& task, const world::WorldModel& world);

TaskValidationResult validate_tasks(
    const std::vector<Task>& tasks,
    const std::vector<agent::AgentSpec>& agents,
    const world::WorldModel& world,
    bool enforce_one_active_task_per_agent = true
);

}  // namespace skymapf::task
