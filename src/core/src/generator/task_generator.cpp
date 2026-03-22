/**
 * @file task_generator.cpp
 * @brief Implements multi-agent task generation.
 */
#include "skymapf/generator/task_generator.hpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>

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

task::TaskModel TaskGenerator::generate(
    const world::WorldModel& world_model,
    const TaskGenerationRequest& request
) {
    if (request.agent_ids.empty()) {
        throw std::invalid_argument("Task generation requires at least one agent id.");
    }

    auto walkable = collect_walkable_cells(world_model);
    if (walkable.empty()) {
        throw std::runtime_error("World has no walkable cell for task generation.");
    }

    auto route_strategy = request.route_strategy;
    if (!route_strategy) {
        route_strategy = std::make_shared<RandomReachableRouteSamplingStrategy>();
    }

    std::mt19937_64 rng(request.random_seed);
    std::shuffle(walkable.begin(), walkable.end(), rng);

    task::TaskModel task_data = task::TaskModel::create(request.task_id, request.name);
    for (std::size_t i = 0; i < request.agent_ids.size(); ++i) {
        const auto start = walkable[i % walkable.size()];
        auto seq = route_strategy->sample(
            world_model,
            start,
            request.waypoint_count_per_agent,
            request.random_seed + static_cast<std::uint64_t>(i * 19937)
        );
        if (!seq.has_minimum_points()) {
            throw std::runtime_error("Route strategy failed to produce a valid visit sequence.");
        }
        task_data.upsert_agent_sequence(
            request.agent_ids[i],
            std::move(seq),
            request.start_time
        );
    }

    return task_data;
}

std::vector<task::TaskModel> TaskGenerator::generate_family(
    const world::WorldModel& world_model,
    const TaskFamilyGenerationRequest& request
) {
    if (request.task_count == 0) {
        throw std::invalid_argument("Task family generation requires task_count > 0.");
    }

    std::vector<task::TaskModel> tasks;
    tasks.reserve(request.task_count);
    for (std::size_t i = 0; i < request.task_count; ++i) {
        TaskGenerationRequest one = request.base_request;
        one.task_id = request.first_task_id + static_cast<common::TaskId>(i);
        one.start_time = request.first_start_time + static_cast<common::TimeStep>(i * request.start_time_step);
        if (one.name.empty()) {
            one.name = request.name_prefix + std::to_string(i);
        }
        one.random_seed = request.base_request.random_seed + static_cast<std::uint64_t>(i * 100003ull);

        tasks.push_back(generate(world_model, one));
    }
    return tasks;
}

std::vector<task::TaskModel> TaskGenerator::generate_family(
    const world::WorldModel& world_model,
    const std::vector<TaskGenerationRequest>& requests
) {
    if (requests.empty()) {
        throw std::invalid_argument("Task family generation requires at least one request item.");
    }

    std::vector<task::TaskModel> tasks;
    tasks.reserve(requests.size());
    for (std::size_t i = 0; i < requests.size(); ++i) {
        tasks.push_back(generate(world_model, requests[i]));
    }
    return tasks;
}

}  // namespace skymapf::generator
