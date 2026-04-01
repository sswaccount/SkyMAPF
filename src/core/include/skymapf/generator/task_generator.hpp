/**
 * @file task_generator.hpp
 * @brief Defines options and APIs for procedural task generation.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "skymapf/common/ids.hpp"
#include "skymapf/generator/strategy/route_sampling_strategy.hpp"
#include "skymapf/task/model.hpp"
#include "../world/model.hpp"

namespace skymapf::generator {

/**
 * @brief Options object for multi-agent task generation.
 *
 * The generator enforces sequence-length and start-time constraints.
 */
struct TaskGenerationOptions {
    /**
     * @brief Constructs options with default task generation settings.
     */
    TaskGenerationOptions() = default;

    /**
     * @brief Constructs options with explicit agent count and random seed.
     *
     * @param count Number of agents to generate in one task.
     * @param seed Random seed used by generation routines.
     */
    explicit TaskGenerationOptions(
        std::size_t count
    )
        : agent_count(count) {}

    /**
     * @brief Constructs options with explicit sequence and start-time constraints.
     *
     * @param count Number of agents to generate in one task.
     * @param seed Random seed used by generation routines.
     * @param max_seq_size Maximum allowed visit-sequence size per agent.
     * @param max_start Latest allowed start time for generated agents.
     */
    TaskGenerationOptions(
        std::size_t count,
        std::uint32_t max_seq_size,
        common::TimeStep max_start
    )
        : agent_count(count),
          max_sequence_size(max_seq_size),
          max_start_time(max_start) {}

    /**
     * @brief Constructs options with explicit route-sampling strategy.
     *
     * @param count Number of agents to generate in one task.
     * @param seed Random seed used by generation routines.
     * @param max_seq_size Maximum allowed visit-sequence size per agent.
     * @param max_start Latest allowed start time for generated agents.
     * @param sampling_strategy Route sampling strategy object.
     */
    TaskGenerationOptions(
        std::size_t count,
        std::uint32_t max_seq_size,
        common::TimeStep max_start,
        std::shared_ptr<IRouteSamplingStrategy> sampling_strategy
    )
        : agent_count(count),
          max_sequence_size(max_seq_size),
          max_start_time(max_start),
          route_sampling_strategy(std::move(sampling_strategy)) {}

    /// Number of agents to generate in one task.
    std::size_t agent_count{1};
    /// Optional explicit task id. When missing, generator auto-completes it.
    std::optional<common::TaskId> task_id;
    /// Optional explicit task name. When missing, generator auto-completes it.
    std::optional<std::string> task_name;
    /// Maximum allowed visit-sequence size for each generated agent.
    std::uint32_t max_sequence_size{2};
    /// Maximum allowed generated start time for each agent.
    common::TimeStep max_start_time{0};
    /// Occupancy behavior applied after generated agents reach goal.
    task::GoalArrivalBehavior goal_arrival_behavior{task::GoalArrivalBehavior::StayAtGoal};
    /// Optional route sampling strategy; default strategy is used when null.
    std::shared_ptr<IRouteSamplingStrategy> route_sampling_strategy;
};

/// Utility for producing multi-agent tasks from world state.
class TaskGenerator {
public:
    /**
     * @brief Generates one multi-agent task using the provided strategy.
     *
     * @param world World used as route sampling domain.
     * @param options Task generation options.
     * @return Generated task object.
     * @throws std::invalid_argument If options are invalid.
     * @throws std::runtime_error If generation or validation fails.
     */
    static task::TaskModel generate(
        const world::WorldModel& world,
        const TaskGenerationOptions& options
    );

    /**
     * @brief Generates multiple tasks by applying one options item per task.
     *
     * @param world World used as route sampling domain.
     * @param options Task generation options list.
     * @return Generated task list preserving input options order.
     * @throws std::invalid_argument If options list is empty.
     * @throws std::runtime_error If any task generation or validation fails.
     */
    static std::vector<task::TaskModel> generate(
        const world::WorldModel& world,
        const std::vector<TaskGenerationOptions>& options
    );

};

}  // namespace skymapf::generator
