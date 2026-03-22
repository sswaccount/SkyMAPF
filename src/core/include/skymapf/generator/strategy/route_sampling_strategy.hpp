/**
 * @file route_sampling_strategy.hpp
 * @brief Defines route sampling strategy interfaces for task generation.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "skymapf/task/model.hpp"
#include "skymapf/world/model.hpp"

namespace skymapf::generator {

struct TaskGenerationOptions;

/// Context visible to route sampling strategies.
struct RouteSamplingContext {
    const world::WorldModel& world;
    const TaskGenerationOptions& options;
    std::size_t agent_index{0};
    common::AgentId agent_id{0};
    common::CellIndex start_cell{0};
    const task::TaskModel& partial_task;
    std::uint64_t agent_seed{0};
};

/// Result returned by route sampling strategies.
struct RouteSamplingResult {
    bool success{false};
    task::SequenceModel sequence;
    std::string error_message;
};

/// Strategy interface for sampling per-agent route checkpoints.
class IRouteSamplingStrategy {
public:
    virtual ~IRouteSamplingStrategy() = default;

    virtual RouteSamplingResult sample(
        const RouteSamplingContext& context
    ) const = 0;
};

/// Random route sampler over currently walkable world cells.
class RandomReachableRouteSamplingStrategy final : public IRouteSamplingStrategy {
public:
    RouteSamplingResult sample(
        const RouteSamplingContext& context
    ) const override;
};

using DefaultRouteSamplingStrategy = RandomReachableRouteSamplingStrategy;

}  // namespace skymapf::generator
