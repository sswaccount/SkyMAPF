/**
 * @file instance_generator.hpp
 * @brief Defines orchestration helpers for one-step instance generation.
 */
#pragma once

#include <optional>
#include <string>
#include <utility>

#include "../common/randomization_context.hpp"
#include "../instance/model.hpp"
#include "./task_generator.hpp"
#include "./world_generator.hpp"

namespace skymapf::generator {

/**
 * @brief Options object for one-step instance generation.
 */
struct InstanceGeneratorOptions {
    WorldGenerationOptions world_gen_options{};
    TaskGenerationOptions task_gen_options{};
    std::optional<common::InstanceId> instance_id;
    std::optional<std::string> instance_name;
    std::optional<common::RandomizationContext> randomization;

    /**
     * @brief Constructs options with default world/task generation settings.
     */
    InstanceGeneratorOptions() = default;

    /**
     * @brief Constructs options with explicit world and task generation options.
     *
     * @param world_options World generation options.
     * @param task_options Task generation options.
     */
    InstanceGeneratorOptions(
        WorldGenerationOptions world_options,
        TaskGenerationOptions task_options
    )
        : world_gen_options(std::move(world_options)),
          task_gen_options(std::move(task_options)) {}

    /**
     * @brief Constructs options with explicit world/task settings and instance identity.
     *
     * @param world_options World generation options.
     * @param task_options Task generation options.
     * @param id Instance identifier used in output model.
     * @param name Optional instance display name.
     */
    InstanceGeneratorOptions(
        WorldGenerationOptions world_options,
        TaskGenerationOptions task_options,
        common::InstanceId id,
        std::string name
    )
        : world_gen_options(std::move(world_options)),
          task_gen_options(std::move(task_options)),
          instance_id(id),
          instance_name(std::move(name)) {}
};

/// Utility that orchestrates one-step instance generation.
class InstanceGenerator {
public:
    /**
     * @brief Generates one instance model by chaining world and task generation.
     *
     * Flow:
     * 1. Generate world from world_gen_options.
     * 2. Generate one task from task_gen_options.
     * 3. Pack world and task into one InstanceModel.
     *
     * @param options Instance generation options.
     * @return Generated instance model.
     */
    static instance::InstanceModel generate(const InstanceGeneratorOptions& options);
};

}  // namespace skymapf::generator