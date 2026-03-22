/**
 * @file scenario_generator.hpp
 * @brief Defines orchestration helpers for world/task/scenario generation.
 */
#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../common/randomization_context.hpp"
#include "../scenario/model.hpp"
#include "./world_generator.hpp"
#include "./task_generator.hpp"


namespace skymapf::generator {

/**
 * @brief Options object for one-step scenario generation.
 */
struct ScenarioGeneratorOptions {
    WorldGenerationOptions world_gen_options{};
    std::vector<TaskGenerationOptions> tasks_gen_options{};
    std::optional<common::ScenarioId> scenario_id;
    std::optional<std::string> scenario_name;
    std::optional<common::RandomizationContext> randomization;

    /**
     * @brief Constructs options with default world/task generation settings.
     */
    ScenarioGeneratorOptions() = default;

    /**
     * @brief Constructs options with explicit world and task generation options.
     *
     * @param world_options World generation options.
     * @param task_options Task generation options.
     */
    ScenarioGeneratorOptions(
        WorldGenerationOptions world_options,
        std::vector<TaskGenerationOptions> tasks_options
    )
        : world_gen_options(std::move(world_options)),
          tasks_gen_options(std::move(tasks_options)) {}
};

/// Utility that orchestrates one-step scenario generation.
class ScenarioGenerator {
public:
    /**
     * @brief Generates one scenario model by chaining world and task generation.
     *
     * Flow:
     * 1. Generate world from world_gen_options.
     * 2. Generate one task from task_gen_options.
     * 3. Pack world and task into one ScenarioModel.
     *
     * @param options Scenario generation options.
     * @return Generated scenario model.
     */
    static scenario::ScenarioModel generate(const ScenarioGeneratorOptions& options);
};

}  // namespace skymapf::generator
