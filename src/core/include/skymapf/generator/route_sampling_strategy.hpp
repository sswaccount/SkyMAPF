/**
 * @file route_sampling_strategy.hpp
 * @brief Defines route sampling strategy interfaces for task generation.
 */
#pragma once

#include <cstddef>
#include <cstdint>

#include "../task/task_model.hpp"
#include "../world/world_model.hpp"

namespace skymapf::generator {

/// Strategy interface for sampling per-agent route checkpoints.
class IRouteSamplingStrategy {
public:
    virtual ~IRouteSamplingStrategy() = default;

    virtual task::SequenceModel sample(
        const world::WorldModel& world,
        common::CellIndex start,
        std::size_t waypoint_count,
        std::uint64_t random_seed
    ) const = 0;
};

/// Random route sampler over currently walkable world cells.
class RandomReachableRouteSamplingStrategy final : public IRouteSamplingStrategy {
public:
    task::SequenceModel sample(
        const world::WorldModel& world,
        common::CellIndex start,
        std::size_t waypoint_count,
        std::uint64_t random_seed
    ) const override;
};

using DefaultRouteSamplingStrategy = RandomReachableRouteSamplingStrategy;

}  // namespace skymapf::generator
