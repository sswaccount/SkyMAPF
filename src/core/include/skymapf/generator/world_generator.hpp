/**
 * @file world_generator.hpp
 * @brief Defines strategies and APIs for procedural world generation.
 */
#pragma once

#include <memory>
#include <utility>

#include "../common/space.hpp"
#include "./edge_generation_strategy.hpp"
#include "./obstacle_layout_strategy.hpp"
#include "../world/validation.hpp"
#include "../world/world_model.hpp"

namespace skymapf::generator {

/**
 * @brief Request object for connected world generation.
 *
 * The generator always validates connectivity after obstacle placement.
 */
struct WorldGenerationRequest {
    /**
     * @brief Constructs a request with default world shape and obstacle settings.
     */
    WorldGenerationRequest() = default;

    /**
     * @brief Constructs a request with explicit space and obstacle settings.
     *
     * @param spec Discrete world dimensions.
     * @param obstacle Density and randomization parameters.
     */
    explicit WorldGenerationRequest(
        common::SpaceSpec spec,
        ObstacleDensityConfig obstacle = {}
    )
        : space_spec(std::move(spec)), obstacle_config(obstacle) {}

    /**
     * @brief Constructs a request with explicit space, settings, and strategy.
     *
     * @param spec Discrete world dimensions.
     * @param obstacle Density and randomization parameters.
     * @param strategy Obstacle generation strategy object.
     */
    WorldGenerationRequest(
        common::SpaceSpec spec,
        ObstacleDensityConfig obstacle,
        std::shared_ptr<IObstacleLayoutStrategy> strategy
    )
        : space_spec(std::move(spec)),
          obstacle_config(obstacle),
          obstacle_strategy(std::move(strategy)) {}

    /**
     * @brief Constructs a request with explicit space, obstacle, and edge strategies.
     *
     * @param spec Discrete world dimensions.
     * @param obstacle Density and randomization parameters.
     * @param obstacle_gen Obstacle generation strategy object.
     * @param edge_gen Edge generation strategy object.
     */
    WorldGenerationRequest(
        common::SpaceSpec spec,
        ObstacleDensityConfig obstacle,
        std::shared_ptr<IObstacleLayoutStrategy> obstacle_gen,
        std::shared_ptr<IEdgeGenerationStrategy> edge_gen
    )
        : space_spec(std::move(spec)),
          obstacle_config(obstacle),
          obstacle_strategy(std::move(obstacle_gen)),
          edge_strategy(std::move(edge_gen)) {}

    /// Discrete world dimensions used for generation.
    common::SpaceSpec space_spec{common::SpaceSpec::make_2d(16, 16)};
    /// Obstacle density and random seed controls.
    ObstacleDensityConfig obstacle_config{};
    /// Optional obstacle strategy; default connected-carving strategy is used when null.
    std::shared_ptr<IObstacleLayoutStrategy> obstacle_strategy;
    /// Optional edge strategy; default bidirectional geometric strategy is used when null.
    std::shared_ptr<IEdgeGenerationStrategy> edge_strategy;
};

/**
 * @brief Generates a connected world using obstacle and edge strategies.
 */
class WorldGenerator {
public:
    /**
     * @brief Generates one validated connected world from request settings.
     *
     * @param request World generation options and strategy objects.
     * @return Generated world model.
     * @throws std::runtime_error If obstacle placement fails or connectivity validation fails.
     */
    static world::WorldModel generate(const WorldGenerationRequest& request);
};

}  // namespace skymapf::generator
