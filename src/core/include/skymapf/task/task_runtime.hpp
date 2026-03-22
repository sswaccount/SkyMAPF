/**
 * @file task_runtime.hpp
 * @brief Defines runtime execution state models for task processing.
 */
#pragma once

#include <cstddef>
#include <vector>

#include "skymapf/common/ids.hpp"

namespace skymapf::task {

/// Represents runtime lifecycle states of one agent assignment.
enum class AgentExecutionStatus {
    Pending,
    Active,
    Completed,
    Failed,
};

/// Runtime progress for one agent assignment in one task.
struct AgentTaskRuntimeState {
    common::AgentId agent_id{0};
    AgentExecutionStatus status{AgentExecutionStatus::Pending};
    std::size_t checkpoint_cursor{0};
};

/// Runtime state bound to one task execution.
struct TaskRuntimeState {
    common::TaskId task_id{0};
    common::TimeStep current_time{0};
    std::vector<AgentTaskRuntimeState> agent_states;
};

}  // namespace skymapf::task
