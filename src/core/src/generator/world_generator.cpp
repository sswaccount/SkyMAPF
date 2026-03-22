/**
 * @file world_generator.cpp
 * @brief Implements obstacle layout and connectivity-aware world generation.
 */
#include "skymapf/generator/world_generator.hpp"

#include <stdexcept>
#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/id_generator.hpp"
#include "skymapf/world/validation.hpp"

namespace skymapf::generator {

world::WorldModel WorldGenerator::generate(const WorldGenerationOptions& options) {
    const auto randomization = options.randomization.value_or(
        options.obstacle_config.random_seed == 0
            ? common::RandomizationContext::make_default()
            : common::RandomizationContext::make_deterministic(options.obstacle_config.random_seed)
    );
    const auto world_id = options.world_id.value_or(
        utils::IdGenerator::deterministic_world_id(randomization)
    );
    const auto world_name = options.world_name.value_or(
        utils::DefaultNaming::world_name(options.space_spec, randomization, world_id)
    );
    auto world_model = world::WorldModel(options.space_spec, {}, {}, world_id, world_name);
    auto obstacle_strategy = options.obstacle_strategy;
    if (!obstacle_strategy) {
        obstacle_strategy = std::make_shared<DefaultObstacleLayoutStrategy>();
    }
    auto edge_strategy = options.edge_strategy;
    if (!edge_strategy) {
        edge_strategy = std::make_shared<DefaultEdgeGenerationStrategy>();
    }

    auto obstacle_config = options.obstacle_config;
    if (options.randomization.has_value() || obstacle_config.random_seed == 0) {
        obstacle_config.random_seed = randomization.derive_seed(
            common::RandomizationDomain::GeneratorObstacle,
            world_id
        );
    }
    if (!obstacle_strategy->apply(world_model, obstacle_config)) {
        throw std::runtime_error("World generation failed: obstacle strategy could not produce a layout.");
    }
    edge_strategy->apply(world_model);

    const auto connectivity_status = world::ConnectivityValidator::validate(world_model);
    if (connectivity_status != world::ConnectivityValidationStatus::Success) {
        throw std::runtime_error("World generation failed: generated world is not connected.");
    }

    return world_model;
}

}  // namespace skymapf::generator
