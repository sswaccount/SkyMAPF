/**
 * @file scenario_generator.cpp
 * @brief Implements orchestration from generation options to scenario model.
 */
#include "skymapf/generator/scenario_generator.hpp"

#include <vector>

#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/id_generator.hpp"

namespace skymapf::generator {

namespace {

common::RandomizationContext resolve_randomization(const ScenarioGeneratorOptions& options) {
    if (options.randomization.has_value()) {
        return *options.randomization;
    }
    if (options.world_gen_options.randomization.has_value()) {
        return *options.world_gen_options.randomization;
    }
    for (const auto& task_options : options.tasks_gen_options) {
        if (task_options.randomization.has_value()) {
            return *task_options.randomization;
        }
        if (task_options.random_seed != 0) {
            return common::RandomizationContext::make_deterministic(task_options.random_seed);
        }
    }
    if (options.world_gen_options.obstacle_config.random_seed != 0) {
        return common::RandomizationContext::make_deterministic(
            options.world_gen_options.obstacle_config.random_seed
        );
    }
    return common::RandomizationContext::make_default();
}

}  // namespace

scenario::ScenarioModel ScenarioGenerator::generate(
    const ScenarioGeneratorOptions& options
) {
    const auto randomization = resolve_randomization(options);
    auto world_options = options.world_gen_options;
    if (!world_options.randomization.has_value()) {
        world_options.randomization = randomization;
    }
    auto task_options = options.tasks_gen_options;
    for (auto& one_task_options : task_options) {
        if (!one_task_options.randomization.has_value()) {
            if (one_task_options.random_seed != 0) {
                one_task_options.randomization =
                    common::RandomizationContext::make_deterministic(one_task_options.random_seed);
            } else {
                one_task_options.randomization = randomization;
            }
        }
    }

    auto world_model = WorldGenerator::generate(world_options);
    auto task_models = TaskGenerator::generate(world_model, task_options);
    const auto scenario_id = options.scenario_id.value_or(
        utils::IdGenerator::deterministic_scenario_id(randomization, world_model.id())
    );
    auto scenario_name = options.scenario_name.value_or(
        utils::DefaultNaming::scenario_name(randomization, scenario_id)
    );

    return scenario::ScenarioModel::create(
        scenario_id,
        std::move(scenario_name),
        std::move(world_model),
        std::move(task_models)
    );
}

}  // namespace skymapf::generator
