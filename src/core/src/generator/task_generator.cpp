/**
 * @file task_generator.cpp
 * @brief Implements multi-agent task generation.
 */
#include "skymapf/generator/task_generator.hpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

#include "skymapf/task/validation.hpp"
#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/random_tool.hpp"

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

    auto walkable = collect_walkable_cells(world_model);
    if (walkable.empty()) {
        throw std::runtime_error("World has no walkable cell for task generation.");
    }

    auto route_strategy = options.route_sampling_strategy;
    if (!route_strategy) {
        route_strategy = std::make_shared<DefaultRouteSamplingStrategy>();
    }

    auto& random_tool = utils::RandomTool::instance();
    std::mt19937_64 task_rng;
    std::mt19937_64 agent_id_rng;
    if (options.random_seed == 0) {
        task_rng.seed(static_cast<std::uint64_t>(random_tool.generator_rng()()));
        agent_id_rng.seed(static_cast<std::uint64_t>(random_tool.id_rng()()));
    } else {
        task_rng.seed(options.random_seed);
        agent_id_rng.seed(options.random_seed ^ 0x9E3779B97F4A7C15ULL);
    }
    std::shuffle(walkable.begin(), walkable.end(), task_rng);
    std::uniform_int_distribution<common::TimeStep> start_time_dist(0, options.max_start_time);

    const auto generated_task_id = options.task_id.value_or(
        static_cast<common::TaskId>(agent_id_rng())
    );
    const auto generated_task_name = options.task_name.value_or(
        utils::DefaultNaming::next_task_name(options.agent_count)
    );
    auto task_data = task::TaskModel::create(
        generated_task_id,
        generated_task_name
    );

    for (std::size_t i = 0; i < options.agent_count; ++i) {
        const auto start = walkable[i % walkable.size()];
        const auto agent_id = static_cast<common::AgentId>(agent_id_rng());
        RouteSamplingContext context{
            world_model,
            options,
            i,
            agent_id,
            start,
            task_data,
            static_cast<std::uint64_t>(task_rng())
        };
        auto sampling_result = route_strategy->sample(context);
        if (!sampling_result.success) {
            throw std::runtime_error(
                sampling_result.error_message.empty()
                    ? "Route sampling strategy failed."
                    : sampling_result.error_message
            );
        }

        const auto seq_size = sampling_result.sequence.checkpoints.size();
        if (seq_size < 2 || seq_size > options.max_sequence_size) {
            throw std::runtime_error(
                "Route sampling strategy returned sequence outside [2, max_sequence_size]."
            );
        }
        task_data.upsert_agent_sequence(
            agent_id,
            std::move(sampling_result.sequence),
            start_time_dist(task_rng)
        );
    }

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
