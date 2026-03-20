/**
 * @file connectivity.hpp
 * @brief Defines connectivity policy abstractions for world neighbors.
 */
#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "../common/ids.hpp"

namespace skymapf::world {

class WorldModel;

/**
 * @brief Abstract policy for querying traversable neighbor cells.
 */
class IConnectivityPolicy {
public:
    virtual ~IConnectivityPolicy() = default;

    /**
     * @brief Returns neighbor indices reachable from a source cell.
     *
     * @param world World used for bounds and walkability checks.
     * @param from Source cell index.
     * @return Neighbor indices valid under this connectivity policy.
     */
    virtual std::vector<common::CellIndex> neighbors(
        const WorldModel& world,
        common::CellIndex from
    ) const = 0;
};

/// Default grid policy using axis-aligned adjacency and walkability checks.
class GridAdjacencyPolicy final : public IConnectivityPolicy {
public:
    std::vector<common::CellIndex> neighbors(
        const WorldModel& world,
        common::CellIndex from
    ) const override;
};

/**
 * @brief Explicit adjacency-list connectivity policy.
 *
 * This policy allows arbitrary graph edges independent of geometric adjacency.
 */
class ExplicitGraphConnectivityPolicy final : public IConnectivityPolicy {
public:
    /// Adds a directed edge from @p from to @p to.
    void add_directed_edge(common::CellIndex from, common::CellIndex to);
    /// Adds both directed edges (a -> b) and (b -> a).
    void add_bidirectional_edge(common::CellIndex a, common::CellIndex b);
    /// Removes all explicit edges.
    void clear() noexcept;

    std::vector<common::CellIndex> neighbors(
        const WorldModel& world,
        common::CellIndex from
    ) const override;

private:
    std::unordered_map<common::CellIndex, std::vector<common::CellIndex>> adjacency_;
};

/// Shared ownership handle for connectivity policy objects.
using ConnectivityPolicyPtr = std::shared_ptr<IConnectivityPolicy>;

}  // namespace skymapf::world
