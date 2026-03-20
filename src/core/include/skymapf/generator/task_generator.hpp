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

class RandomReachableRouteSamplingStrategy final : public IRouteSamplingStrategy {
public:
    task::VisitSequence sample(
        const world::WorldModel& world,
        common::CellIndex start,
        std::size_t waypoint_count,
        std::uint64_t random_seed
    ) const override;
};

struct TaskGenerationRequest {
    common::TaskId task_id{1};
    std::vector<common::AgentId> agent_ids;
    std::size_t waypoint_count_per_agent{0};
    task::TaskTiming timing{};
    std::string name;
    std::uint64_t random_seed{0};
    std::shared_ptr<IRouteSamplingStrategy> route_strategy;
};

struct TaskGenerationResult {
    std::optional<task::Task> task;
    std::string error_message;
};

class TaskGenerator {
public:
    static TaskGenerationResult generate(
        const world::WorldModel& world,
        const TaskGenerationRequest& request
    );
};

}  // namespace skymapf::generator
