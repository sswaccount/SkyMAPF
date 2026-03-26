/**
 * @file world_generator.hpp
 * @brief Defines strategies and APIs for procedural world generation.
 */
#pragma once

#include <memory>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "../common/space.hpp"
#include "./strategy/edge_generation_strategy.hpp"
#include "./strategy/obstacle_layout_strategy.hpp"
#include "../world/validation.hpp"
#include "../world/model.hpp"

namespace skymapf::generator {

/**
 * @brief Options object for connected world generation.
 *
 * The generator always validates connectivity after obstacle placement.
 */
struct WorldGenerationOptions {
    /**
     * @brief Constructs options with default world shape and obstacle settings.
     */
    WorldGenerationOptions() = default;

    /**
     * @brief Constructs options with explicit space and obstacle density.
     *
     * @param spec Discrete world dimensions.
     * @param density Obstacle ratio in [0, 1).
     */
    explicit WorldGenerationOptions(
        common::SpaceSpec spec,
        double density = 0.2
    )
        : space_spec(std::move(spec)), obstacle_density(density) {}

    /**
     * @brief Constructs options with explicit space, density, and edge strategies.
     *
     * @param spec Discrete world dimensions.
     * @param density Obstacle ratio in [0, 1).
     * @param obstacle_gen Obstacle generation strategy object.
     * @param edge_gen Edge generation strategy object.
     */
    WorldGenerationOptions(
        common::SpaceSpec spec,
        double density,
        std::shared_ptr<IObstacleLayoutStrategy> obstacle_gen,
        std::shared_ptr<IEdgeGenerationStrategy> edge_gen
    )
        : space_spec(std::move(spec)),
          obstacle_density(density),
          obstacle_strategy(std::move(obstacle_gen)),
          edge_strategy(std::move(edge_gen)) {}

    /// Discrete world dimensions used for generation.
    common::SpaceSpec space_spec{common::SpaceSpec::make_2d(16, 16)};
    /// Optional explicit world id. When missing, generator auto-completes it.
    std::optional<common::WorldId> world_id;
    /// Optional explicit world name. When missing, generator auto-completes it.
    std::optional<std::string> world_name;
    /// Optional deterministic seed for world generation.
    std::uint64_t random_seed{0};
    /// Obstacle ratio in [0, 1).
    double obstacle_density{0.2};
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
     * @brief Generates one validated connected world from options.
     *
     * @param options World generation options and strategy objects.
     * @return Generated world model.
     * @throws std::runtime_error If obstacle placement fails or connectivity validation fails.
     */
    static world::WorldModel generate(const WorldGenerationOptions& options);
};

}  // namespace skymapf::generator
