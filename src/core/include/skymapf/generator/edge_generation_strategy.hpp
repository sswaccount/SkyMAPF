/**
 * @file edge_generation_strategy.hpp
 * @brief Declares edge generation strategies for world connectivity policies.
 */
#pragma once

#include "../world/world_model.hpp"

namespace skymapf::generator {

/**
 * @brief Strategy interface for constructing connectivity edges in a world.
 */
class IEdgeGenerationStrategy {
public:
    virtual ~IEdgeGenerationStrategy() = default;

    /**
     * @brief Builds or updates the world's connectivity policy.
     *
     * @param world Target world model whose connectivity policy may be replaced.
     */
    virtual void apply(world::WorldModel& world) const = 0;
};

/**
 * @brief Builds bidirectional geometric edges for all in-bounds cell pairs.
 *
 * The strategy constructs an explicit graph policy and adds bidirectional
 * edges between geometric neighbors (4-neighborhood in 2D, 6-neighborhood in 3D).
 */
class BidirectionalGeometricEdgeGenerationStrategy final : public IEdgeGenerationStrategy {
public:
    /**
     * @brief Applies geometric bidirectional edge construction to @p world.
     *
     * @param world Target world model to update.
     */
    void apply(world::WorldModel& world) const override;
};

/// Alias exposing the library default edge generation strategy name.
using DefaultEdgeGenerationStrategy = BidirectionalGeometricEdgeGenerationStrategy;

}  // namespace skymapf::generator
