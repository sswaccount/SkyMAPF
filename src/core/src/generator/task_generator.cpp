/**
 * @file task_generator.cpp
 * @brief Implements multi-agent task generation.
 */
#include "skymapf/generator/task_generator.hpp"

#include <stdexcept>

#include "skymapf/task/validation.hpp"
#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/random_tool.hpp"

namespace skymapf::generator {

namespace {

void validate_options_or_throw(const TaskGenerationOptions& options) {
    if (options.agent_count == 0) {
        throw std::invalid_argument("agent_count must be greater than 0.");
    }
    if (options.max_sequence_size < 2) {
        throw std::invalid_argument("max_sequence_size must be greater than or equal to 2.");
    }
}

}  // namespace

task::TaskModel TaskGenerator::generate(
    const world::WorldModel& world_model,
    const TaskGenerationOptions& options
) {
    validate_options_or_throw(options);

    auto walkable = world_model.walkable_cells();
    if (walkable.empty()) {
        throw std::runtime_error("World has no walkable cell for task generation.");
    }

    const auto task_id = options.task_id.value_or(
        static_cast<common::TaskId>(utils::RandomTool::instance().next_id_value())
    );
    const auto task_name = options.task_name.value_or(
        utils::DefaultNaming::next_task_name(options.agent_count)
    );
    auto task_data = task::TaskModel(
        task_id,
        task_name
    );
    auto route_strategy = options.route_sampling_strategy;
    if (!route_strategy) {
        route_strategy = std::make_shared<DefaultRouteSamplingStrategy>();
    }

    const RouteSamplingContext context{
        options.agent_count,
        options.max_sequence_size,
        options.max_start_time,
        options.goal_arrival_behavior
    };
    route_strategy->apply(world_model, task_data, context);

    const auto validation = task::validate_generated_task(task_data, world_model, options);
    if (!validation.ok) {
        if (!validation.issues.empty()) {
            throw std::runtime_error("Task validation failed: " + validation.issues.front().message);
        }
        throw std::runtime_error("Task validation failed.");
    }

    return task_data;
}

std::vector<task::TaskModel> TaskGenerator::generate(
    const world::WorldModel& world_model,
    const std::vector<TaskGenerationOptions>& options
) {
    if (options.empty()) {
        throw std::invalid_argument("Task batch generation requires at least one options item.");
    }

    std::vector<task::TaskModel> tasks;
    tasks.reserve(options.size());
    for (const auto& one_options : options) {
        tasks.push_back(generate(world_model, one_options));
    }
    return tasks;
}

}  // namespace skymapf::generator
