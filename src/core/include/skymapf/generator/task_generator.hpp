/**
 * @file task_generator.hpp
 * @brief Defines task generation request/result models and generator APIs.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "skymapf/generator/route_sampling_strategy.hpp"
#include "../task/task_model.hpp"
#include "../world/world_model.hpp"

namespace skymapf::generator {

/// Input parameters for constructing one multi-agent task.
struct TaskGenerationRequest {
    common::TaskId task_id{1};
    std::vector<common::AgentId> agent_ids;
    std::size_t waypoint_count_per_agent{0};
    common::TimeStep start_time{0};
    std::string name;
    std::uint64_t random_seed{0};
    std::shared_ptr<IRouteSamplingStrategy> route_strategy;
};

/// Input parameters for constructing a family of tasks from one world.
struct TaskFamilyGenerationRequest {
    TaskGenerationRequest base_request;
    std::size_t task_count{1};
    common::TaskId first_task_id{1};
    common::TimeStep first_start_time{0};
    common::TimeStep start_time_step{1};
    std::string name_prefix{"task_"};
};

/// Utility for producing multi-agent tasks from world state.
class TaskGenerator {
public:
    /**
     * @brief Generates one multi-agent task using the provided strategy.
     *
     * @param world World used as route sampling domain.
     * @param request Task generation options and agent set.
     * @return Generated task object.
     */
    static task::TaskModel generate(
        const world::WorldModel& world,
        const TaskGenerationRequest& request
    );

    /**
     * @brief Generates multiple tasks from one world using a shared base request.
     *
     * @param world World used as route sampling domain.
     * @param request Task-family generation options.
     * @return Generated task list.
     */
    static std::vector<task::TaskModel> generate_family(
        const world::WorldModel& world,
        const TaskFamilyGenerationRequest& request
    );

    /**
     * @brief Generates multiple tasks from one world using per-task requests.
     *
     * @param world World used as route sampling domain.
     * @param requests Explicit request list, one item per task.
     * @return Generated task list.
     */
    static std::vector<task::TaskModel> generate_family(
        const world::WorldModel& world,
        const std::vector<TaskGenerationRequest>& requests
    );
};

}  // namespace skymapf::generator
