/**
 * @file model.hpp
 * @brief Defines the mutable discrete world model used by planning.
 */
#pragma once

#include <functional>
#include <string>
#include <utility>
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
    /// Factory function for creating occupancy maps.
    using OccupancyFactory = std::function<OccupancyStorePtr(common::CellIndex)>;
    /// Factory function for creating connectivity policies.
    using ConnectivityFactory = std::function<ConnectivityGraphPtr()>;

    /// Constructs a default 1x1 walkable 2D world.
    explicit WorldModel(common::SpaceSpec space_spec);

    /**
     * @brief Constructs a world with explicit shape and pluggable factories.
     *
     * @param space_spec Discrete world dimensions.
     * @param occupancy_factory Factory for creating occupancy storage.
     * @param connectivity_factory Factory for creating connectivity policy.
     * @param id Optional explicit world id; auto-generated when missing.
     * @param name Optional explicit world display name; auto-generated when missing.
     */
     explicit WorldModel(
        common::SpaceSpec space_spec,
        OccupancyFactory occupancy_factory = {},
        ConnectivityFactory connectivity_factory = {}
    );

    /**
     * @brief Constructs a world with explicit shape and pluggable factories.
     *
     * @param space_spec Discrete world dimensions.
     * @param occupancy_factory Factory for creating occupancy storage.
     * @param connectivity_factory Factory for creating connectivity policy.
     * @param id Optional explicit world id; auto-generated when missing.
     * @param name Optional explicit world display name; auto-generated when missing.
     */
    explicit WorldModel(
        common::WorldId id,
        std::string name,
        common::SpaceSpec space_spec,
        OccupancyFactory occupancy_factory = {},
        ConnectivityFactory connectivity_factory = {}
    );

    /// Returns stable world id for program references and serialization.
    const common::WorldId& id() const noexcept { return id_; }
    /// Sets world id (primarily for migration and import workflows).
    void set_id(common::WorldId id) noexcept { id_ = id; }

    /// Returns display name for logging and export.
    const std::string& name() const noexcept { return name_; }
    /// Returns whether display name is non-empty.
    bool has_name() const noexcept { return !name_.empty(); }
    /// Sets display name.
    void set_name(std::string name) { name_ = std::move(name); }

    /// Returns current discrete world dimensions.
    const common::SpaceSpec& space_spec() const noexcept { return space_spec_; }
    /// Returns current occupancy storage object.
    const OccupancyStorePtr& occupancy_policy() const noexcept { return occupancy_policy_; }
    /// Returns current connectivity policy object.
    const ConnectivityGraphPtr& connectivity_policy() const noexcept { return connectivity_policy_; }

    /// Returns the total number of cells in the current space.
    common::CellIndex cell_count() const noexcept;
    /// Returns whether the index refers to a valid world cell.
    bool is_valid_index(common::CellIndex index) const noexcept;
    /// Returns whether a 2D coordinate is valid for this world shape.
    bool is_valid_coord(const common::CellCoord2D& coord) const noexcept;
    /// Returns whether a 3D coordinate is valid for this world shape.
    bool is_valid_coord(const common::CellCoord3D& coord) const noexcept;

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
    /// Returns feasible neighbors for a 2D coordinate.
    std::vector<common::CellIndex> neighbors(const common::CellCoord2D& from) const;
    /// Returns feasible neighbors for a 3D coordinate.
    std::vector<common::CellIndex> neighbors(const common::CellCoord3D& from) const;

    /**
     * @brief Removes all graph edges from the active editable connectivity policy.
     *
     * @return True when edges are cleared; false when active policy is not editable-graph mode.
     */
    bool clear_edges();
    /**
     * @brief Adds one directed edge to the active editable connectivity graph.
     *
     * @return True on success; false when active policy is not editable-graph mode
     *         or either endpoint is invalid.
     */
    bool add_directed_edge(common::CellIndex from, common::CellIndex to);
    /// Adds one directed edge from a 2D source to a 2D target.
    bool add_directed_edge(const common::CellCoord2D& from, const common::CellCoord2D& to);
    /// Adds one directed edge from a 3D source to a 3D target.
    bool add_directed_edge(const common::CellCoord3D& from, const common::CellCoord3D& to);
    /**
     * @brief Adds one bidirectional edge pair to the active editable graph.
     *
     * This is equivalent to adding both (a -> b) and (b -> a).
     *
     * @return True on success; false when active policy is not editable-graph mode
     *         or either endpoint is invalid.
     */
    bool add_bidirectional_edge(common::CellIndex a, common::CellIndex b);
    /// Adds one bidirectional edge pair for 2D coordinates.
    bool add_bidirectional_edge(const common::CellCoord2D& a, const common::CellCoord2D& b);
    /// Adds one bidirectional edge pair for 3D coordinates.
    bool add_bidirectional_edge(const common::CellCoord3D& a, const common::CellCoord3D& b);
    /// Returns whether directed edge (from -> to) exists.
    bool has_directed_edge(common::CellIndex from, common::CellIndex to) const noexcept;
    /// Returns whether directed edge (from -> to) exists for 2D coordinates.
    bool has_directed_edge(const common::CellCoord2D& from, const common::CellCoord2D& to) const noexcept;
    /// Returns whether directed edge (from -> to) exists for 3D coordinates.
    bool has_directed_edge(const common::CellCoord3D& from, const common::CellCoord3D& to) const noexcept;

private:
    common::WorldId id_;
    std::string name_;
    common::SpaceSpec space_spec_;
    OccupancyFactory occupancy_factory_;
    ConnectivityFactory connectivity_factory_;
    OccupancyStorePtr occupancy_policy_;
    ConnectivityGraphPtr connectivity_policy_;
};

}  // namespace skymapf::world
