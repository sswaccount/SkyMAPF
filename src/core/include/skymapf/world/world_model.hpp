/**
 * @file world_model.hpp
 * @brief Defines the mutable discrete world model used by planning.
 */
#pragma once

#include <vector>

#include "../common/coord.hpp"
#include "../common/ids.hpp"
#include "../common/space.hpp"
#include "connectivity.hpp"
#include "occupancy.hpp"

namespace skymapf::world {

/**
 * @brief Represents a mutable world with occupancy and connectivity policies.
 *
 * The world stores discrete dimensions and delegates walkability and
 * neighborhood semantics to pluggable strategy objects.
 */
class WorldModel {
public:
    /// Constructs a default 1x1 walkable 2D world.
    WorldModel();
    /**
     * @brief Constructs a world with explicit shape and default walkability.
     *
     * @param space_spec Discrete world dimensions.
     * @param default_walkable Initial walkability value for all cells.
     */
    explicit WorldModel(common::SpaceSpec space_spec, bool default_walkable = true);

    /// Returns the current discrete space specification.
    const common::SpaceSpec& space_spec() const noexcept { return space_spec_; }
    /**
     * @brief Replaces the world shape and resets occupancy.
     *
     * @param space_spec New space specification.
     * @post Occupancy map is recreated with all cells walkable.
     */
    void set_space_spec(common::SpaceSpec space_spec);

    /// Returns the total number of cells in the current space.
    common::CellIndex cell_count() const noexcept;
    /// Returns whether the index refers to a valid world cell.
    bool is_valid_index(common::CellIndex index) const noexcept;

    /// Returns whether the indexed cell is currently walkable.
    bool is_walkable(common::CellIndex index) const noexcept;
    /// Returns whether the 2D coordinate is in-bounds and walkable.
    bool is_walkable(const common::CellCoord2D& coord) const noexcept;
    /// Returns whether the 3D coordinate is in-bounds and walkable.
    bool is_walkable(const common::CellCoord3D& coord) const noexcept;

    /// Sets walkability by linear cell index.
    void set_walkable(common::CellIndex index, bool walkable);
    /// Sets walkability for a 2D coordinate when valid.
    void set_walkable(const common::CellCoord2D& coord, bool walkable);
    /// Sets walkability for a 3D coordinate when valid.
    void set_walkable(const common::CellCoord3D& coord, bool walkable);

    /**
     * @brief Returns neighbor cells for a source index.
     *
     * Neighborhood semantics are provided by the active connectivity policy.
     */
    std::vector<common::CellIndex> neighbors(common::CellIndex from) const;

    /**
     * @brief Replaces occupancy map implementation when dimensions match.
     *
     * @param occupancy Shared occupancy map implementation.
     */
    void set_occupancy_map(OccupancyMapPtr occupancy);
    /// Returns the currently installed occupancy map.
    const OccupancyMapPtr& occupancy_map() const noexcept { return occupancy_; }

    /// Replaces the connectivity policy when non-null.
    void set_connectivity_policy(ConnectivityPolicyPtr policy);
    /// Returns the currently installed connectivity policy.
    const ConnectivityPolicyPtr& connectivity_policy() const noexcept { return connectivity_policy_; }

private:
    common::SpaceSpec space_spec_;
    OccupancyMapPtr occupancy_;
    ConnectivityPolicyPtr connectivity_policy_;
};

}  // namespace skymapf::world
