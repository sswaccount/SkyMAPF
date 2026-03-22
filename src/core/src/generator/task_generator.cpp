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
#include "skymapf/utils/id_generator.hpp"

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

common::TaskId derive_task_id(
    const world::WorldModel& world_model,
    const TaskGenerationOptions& options,
    const common::RandomizationContext& randomization
) {
    // A stable deterministic id derived from generation context.
    std::uint64_t value = options.random_seed;
    value ^= (static_cast<std::uint64_t>(world_model.cell_count()) << 1);
    value ^= (static_cast<std::uint64_t>(options.agent_count) << 17);
    value ^= (static_cast<std::uint64_t>(options.max_sequence_size) << 33);
    value ^= (static_cast<std::uint64_t>(options.max_start_time) << 49);
    if (value == 0) {
        value = 1;
    }
    return utils::IdGenerator::deterministic_task_id(randomization, value);
}

common::AgentId derive_agent_id(
    const common::RandomizationContext& randomization,
    common::TaskId task_id,
    std::size_t index
) {
    const auto salt = static_cast<std::uint64_t>(task_id) ^ static_cast<std::uint64_t>(index + 1);
    return utils::IdGenerator::deterministic_agent_id(randomization, salt);
}

}  // namespace

task::TaskModel TaskGenerator::generate(
    const world::WorldModel& world_model,
    const TaskGenerationOptions& options
) {
    validate_options_or_throw(options);
    const auto randomization = options.randomization.value_or(
        options.random_seed == 0
            ? common::RandomizationContext::make_default()
            : common::RandomizationContext::make_deterministic(options.random_seed)
    );

    auto walkable = collect_walkable_cells(world_model);
    if (walkable.empty()) {
        throw std::runtime_error("World has no walkable cell for task generation.");
    }

    auto route_strategy = options.route_sampling_strategy;
    if (!route_strategy) {
        route_strategy = std::make_shared<DefaultRouteSamplingStrategy>();
    }

    auto rng = randomization.make_rng(
        common::RandomizationDomain::GeneratorTask,
        static_cast<std::uint64_t>(world_model.id())
    );
    std::shuffle(walkable.begin(), walkable.end(), rng);
    std::uniform_int_distribution<common::TimeStep> start_time_dist(0, options.max_start_time);

    const auto generated_task_id = options.task_id.value_or(derive_task_id(world_model, options, randomization));
    const auto generated_task_name = options.task_name.value_or(
        utils::DefaultNaming::task_name(options.agent_count, randomization, generated_task_id)
    );
    auto task_data = task::TaskModel::create(
        generated_task_id,
        generated_task_name
    );

    for (std::size_t i = 0; i < options.agent_count; ++i) {
        const auto start = walkable[i % walkable.size()];
        const auto agent_id = derive_agent_id(randomization, generated_task_id, i);
        RouteSamplingContext context{
            world_model,
            options,
            i,
            agent_id,
            start,
            task_data,
            randomization.derive_seed(
                common::RandomizationDomain::GeneratorRoute,
                static_cast<std::uint64_t>(generated_task_id) ^ static_cast<std::uint64_t>(i + 1)
            )
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
            start_time_dist(rng)
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
