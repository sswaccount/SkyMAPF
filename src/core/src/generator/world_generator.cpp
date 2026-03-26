/**
 * @file world_generator.cpp
 * @brief Implements obstacle layout and connectivity-aware world generation.
 */
#include "skymapf/generator/world_generator.hpp"

#include <random>
#include <stdexcept>
#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/random_tool.hpp"
#include "skymapf/world/validation.hpp"

namespace skymapf::generator {

world::WorldModel WorldGenerator::generate(const WorldGenerationOptions& options) {
    const auto world_id = options.world_id.value_or(
        utils::RandomTool::instance().next_id_value()
    );
    const auto world_name = options.world_name.value_or(
        utils::DefaultNaming::next_world_name(options.space_spec)
    );
    auto world_model = world::WorldModel(world_id, world_name, options.space_spec, {}, {});
    auto obstacle_strategy = options.obstacle_strategy;
    if (!obstacle_strategy) {
        obstacle_strategy = std::make_shared<DefaultObstacleLayoutStrategy>();
    }
    auto edge_strategy = options.edge_strategy;
    if (!edge_strategy) {
        edge_strategy = std::make_shared<DefaultEdgeGenerationStrategy>();
    }

    const auto obstacle_seed = options.random_seed == 0
        ? static_cast<std::uint64_t>(utils::RandomTool::instance().generator_rng()())
        : options.random_seed;
    if (!obstacle_strategy->apply(world_model, options.obstacle_density, obstacle_seed)) {
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
