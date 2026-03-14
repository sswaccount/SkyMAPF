/**
 * @file route_sampling_strategy.hpp
 * @brief Defines route sampling strategy interfaces for task generation.
 */
#pragma once

#include <cstddef>
#include <cstdint>

#include "skymapf/task/model.hpp"
#include "skymapf/world/model.hpp"

namespace skymapf::generator {

/**
 * @brief Context visible to route sampling strategies.
 *
 * The context carries generation limits shared by built-in and custom
 * route sampling implementations.
 */
struct RouteSamplingContext {
    std::size_t agent_count{1};          ///< Number of agents to generate.
    std::uint32_t max_sequence_size{2};  ///< Maximum checkpoints per generated sequence.
    common::TimeStep max_start_time{0};  ///< Upper bound of randomized start time.
    task::GoalArrivalBehavior goal_arrival_behavior{task::GoalArrivalBehavior::StayAtGoal};
    ///< Occupancy behavior assigned when agent reaches goal.
};

/// Strategy interface for sampling per-agent route checkpoints.
class IRouteSamplingStrategy {
public:
    virtual ~IRouteSamplingStrategy() = default;

    /**
     * @brief Generates and writes agent route entries into @p task.
     *
     * Implementations are expected to append or update per-agent sequences
     * inside @p task using the provided world and context constraints.
     *
     * @param world World used for walkability and neighborhood queries.
     * @param task Mutable task model to populate.
     * @param context Route generation limits.
     */
    virtual void apply(
        const world::WorldModel& world,
        task::TaskModel& task,
        const RouteSamplingContext& context
    ) const = 0;
};

/// Random route sampler over currently walkable world cells.
class RandomReachableRouteSamplingStrategy final : public IRouteSamplingStrategy {
public:
    /**
     * @brief Populates @p task with randomized reachable routes.
     *
     * For each agent, this strategy samples one walkable start cell, performs
     * a random walk over graph neighbors to build checkpoints, and randomizes
     * start time and goal-arrival behavior.
     */
    void apply(
        const world::WorldModel& world,
        task::TaskModel& task,
        const RouteSamplingContext& context
    ) const override;
};

using DefaultRouteSamplingStrategy = RandomReachableRouteSamplingStrategy;

}  // namespace skymapf::generator
