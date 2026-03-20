#include "skymapf/generator/task_generator.hpp"

#include <algorithm>
#include <random>

namespace skymapf::generator {

namespace {

std::vector<common::CellIndex> collect_walkable_cells(const world::WorldModel& world_model) {
    std::vector<common::CellIndex> cells;
    cells.reserve(static_cast<std::size_t>(world_model.cell_count()));
    for (common::CellIndex i = 0; i < world_model.cell_count(); ++i) {
        if (world_model.is_walkable(i)) {
            cells.push_back(i);
        }
    }
    return cells;
}

}  // namespace

task::VisitSequence RandomReachableRouteSamplingStrategy::sample(
    const world::WorldModel& world_model,
    common::CellIndex start,
    std::size_t waypoint_count,
    std::uint64_t random_seed
) const {
    task::VisitSequence seq;
    if (!world_model.is_valid_index(start) || !world_model.is_walkable(start)) {
        return seq;
    }
    auto walkable = collect_walkable_cells(world_model);
    if (walkable.empty()) {
        return seq;
    }

    std::mt19937_64 rng(random_seed);
    std::uniform_int_distribution<std::size_t> pick(0, walkable.size() - 1);

    seq.checkpoints.reserve(waypoint_count + 2);
    seq.checkpoints.push_back(start);
    for (std::size_t i = 0; i < waypoint_count; ++i) {
        seq.checkpoints.push_back(walkable[pick(rng)]);
    }
    seq.checkpoints.push_back(walkable[pick(rng)]);
    return seq;
}

TaskGenerationResult TaskGenerator::generate(
    const world::WorldModel& world_model,
    const TaskGenerationRequest& request
) {
    TaskGenerationResult result;
    if (request.agent_ids.empty()) {
        result.error_message = "Task generation requires at least one agent id.";
        return result;
    }

    auto walkable = collect_walkable_cells(world_model);
    if (walkable.empty()) {
        result.error_message = "World has no walkable cell for task generation.";
        return result;
    }

    auto route_strategy = request.route_strategy;
    if (!route_strategy) {
        route_strategy = std::make_shared<RandomReachableRouteSamplingStrategy>();
    }

    std::mt19937_64 rng(request.random_seed);
    std::shuffle(walkable.begin(), walkable.end(), rng);

    task::Task task_data = task::Task::create(request.task_id, request.timing, request.name);
    for (std::size_t i = 0; i < request.agent_ids.size(); ++i) {
        const auto start = walkable[i % walkable.size()];
        auto seq = route_strategy->sample(
            world_model,
            start,
            request.waypoint_count_per_agent,
            request.random_seed + static_cast<std::uint64_t>(i * 19937)
        );
        if (!seq.has_minimum_points()) {
            result.error_message = "Route strategy failed to produce a valid visit sequence.";
            return result;
        }
        task_data.upsert_agent_sequence(request.agent_ids[i], std::move(seq));
    }

    result.task = std::move(task_data);
    return result;
}

}  // namespace skymapf::generator
