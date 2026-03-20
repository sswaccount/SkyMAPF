/**
 * @file task_generator.hpp
 * @brief Defines task generation strategies and request/result models.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../task/task.hpp"
#include "../world/world_model.hpp"

namespace skymapf::generator {

/// Strategy interface for sampling per-agent route checkpoints.
class IRouteSamplingStrategy {
public:
    virtual ~IRouteSamplingStrategy() = default;

    virtual task::VisitSequence sample(
        const world::WorldModel& world,
        common::CellIndex start,
        std::size_t waypoint_count,
        std::uint64_t random_seed
    ) const = 0;
};

/// Random route sampler over currently walkable world cells.
class RandomReachableRouteSamplingStrategy final : public IRouteSamplingStrategy {
public:
    task::VisitSequence sample(
        const world::WorldModel& world,
        common::CellIndex start,
        std::size_t waypoint_count,
        std::uint64_t random_seed
    ) const override;
};

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

/// Output of task generation including optional error message.
struct TaskGenerationResult {
    std::optional<task::Task> task;
    std::string error_message;
};

/// Utility for producing multi-agent tasks from world state.
class TaskGenerator {
public:
    /**
     * @brief Generates one multi-agent task using the provided strategy.
     *
     * @param world World used as route sampling domain.
     * @param request Task generation options and agent set.
     * @return Task generation result with either task or error text.
     */
    static TaskGenerationResult generate(
        const world::WorldModel& world,
        const TaskGenerationRequest& request
    );
};

}  // namespace skymapf::generator
