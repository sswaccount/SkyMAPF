/**
 * @file world_generator.cpp
 * @brief Implements obstacle layout and connectivity-aware world generation.
 */
#include "skymapf/generator/world_generator.hpp"

#include <stdexcept>
#include "skymapf/world/validation.hpp"

namespace skymapf::generator {

world::WorldModel WorldGenerator::generate(const WorldGenerationRequest& request) {
    auto world_model = world::WorldModel(request.space_spec);
    auto obstacle_strategy = request.obstacle_strategy;
    if (!obstacle_strategy) {
        obstacle_strategy = std::make_shared<DefaultObstacleLayoutStrategy>();
    }
    auto edge_strategy = request.edge_strategy;
    if (!edge_strategy) {
        edge_strategy = std::make_shared<DefaultEdgeGenerationStrategy>();
    }

    if (!obstacle_strategy->apply(world_model, request.obstacle_config)) {
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
