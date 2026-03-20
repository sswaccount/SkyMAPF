/**
 * @file world_generator.hpp
 * @brief Defines strategies and APIs for procedural world generation.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "../common/space.hpp"
#include "../world/world_model.hpp"

namespace skymapf::generator {

/// Parameters controlling obstacle density sampling.
struct ObstacleDensityConfig {
    // Obstacle ratio in [0, 1). 0 means empty map.
    double obstacle_density{0.2};
    std::uint64_t random_seed{0};
};

/**
 * @brief Strategy interface for generating obstacle layouts.
 */
class IObstacleLayoutStrategy {
public:
    virtual ~IObstacleLayoutStrategy() = default;

    // Returns false when the strategy cannot generate a valid layout.
    virtual bool apply(
        world::WorldModel& world,
        const ObstacleDensityConfig& config
    ) const = 0;
};

// Guarantees walkable cells are connected by construction (for grid adjacency worlds).
class ConnectedCarvingObstacleLayoutStrategy final : public IObstacleLayoutStrategy {
public:
    bool apply(
        world::WorldModel& world,
        const ObstacleDensityConfig& config
    ) const override;
};

/**
 * @brief Strategy interface for repairing disconnected walkable regions.
 */
class IConnectivityRepairStrategy {
public:
    virtual ~IConnectivityRepairStrategy() = default;
    virtual void repair(world::WorldModel& world) const = 0;
};

// Keeps only largest walkable component and blocks all other walkable islands.
class KeepLargestComponentRepairStrategy final : public IConnectivityRepairStrategy {
public:
    void repair(world::WorldModel& world) const override;
};

struct WorldGenerationRequest {
    common::SpaceSpec space_spec{common::SpaceSpec::make_2d(16, 16)};
    ObstacleDensityConfig obstacle_config{};
    bool require_single_component{true};
    std::size_t max_attempts{8};
    std::shared_ptr<IObstacleLayoutStrategy> obstacle_strategy;
    std::shared_ptr<IConnectivityRepairStrategy> repair_strategy;
};

/// Result bundle for world generation requests.
struct WorldGenerationResult {
    world::WorldModel world;
    bool connected{true};
    std::size_t attempts_used{1};
};

/**
 * @brief Generates a world using configurable obstacle and repair strategies.
 */
class WorldGenerator {
public:
    /**
     * @brief Generates one world instance from request settings.
     *
     * @param request World generation options and strategy objects.
     * @return Generated world together with connectivity status metadata.
     */
    static WorldGenerationResult generate(const WorldGenerationRequest& request);
};

}  // namespace skymapf::generator
