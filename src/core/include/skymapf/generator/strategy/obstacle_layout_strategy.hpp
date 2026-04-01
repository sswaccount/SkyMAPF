/**
 * @file obstacle_layout_strategy.hpp
 * @brief Declares obstacle layout strategy abstractions for world generation.
 */
#pragma once

#include "skymapf/world/model.hpp"

namespace skymapf::generator {

/**
 * @brief Context visible to obstacle layout strategies.
 */
struct ObstacleLayoutContext {
    double obstacle_density{0.2};  ///< Target obstacle ratio in [0, 1).
};

/**
 * @brief Strategy interface for generating obstacle layouts.
 */
class IObstacleLayoutStrategy {
public:
    virtual ~IObstacleLayoutStrategy() = default;

    /**
     * @brief Applies obstacle layout generation to a world model.
     *
     * Implementations may mutate walkability of any valid cell in @p world.
     *
     * @param world Target world model to mutate.
     * @param context Obstacle generation context and limits.
     * @return True when generation succeeds; false when no valid layout can be produced.
     */
    virtual bool apply(
        world::WorldModel& world,
        const ObstacleLayoutContext context
    ) const = 0;
};

/**
 * @brief Connected-carving obstacle strategy for grid-adjacency worlds.
 *
 * This strategy carves walkable cells from a blocked map and keeps carved cells
 * in one connected region.
 */
class ConnectedCarvingObstacleLayoutStrategy final : public IObstacleLayoutStrategy {
public:
    /**
     * @brief Generates one connected walkable region under the requested density.
     *
     * @param world Target world model to mutate.
     * @param context Obstacle generation context and limits.
     * @return True when at least one walkable cell is carved; false otherwise.
     */
    bool apply(
        world::WorldModel& world,
        const ObstacleLayoutContext context
    ) const override;
};

/// Alias exposing the library default obstacle strategy name.
using DefaultObstacleLayoutStrategy = ConnectedCarvingObstacleLayoutStrategy;

}  // namespace skymapf::generator
