/**
 * @file connectivity.hpp
 * @brief Defines connectivity policy abstractions for world neighbors.
 */
#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../common/ids.hpp"
#include "../common/space.hpp"

namespace skymapf::world {

class WorldModel;

/**
 * @brief Returns in-bounds geometric neighbors for a grid cell index.
 *
 * This helper ignores walkability and returns axis-aligned neighbors only:
 * 4-neighborhood in 2D and 6-neighborhood in 3D.
 */
std::vector<common::CellIndex> geometric_neighbors(
    const common::SpaceSpec& spec,
    common::CellIndex from
);

/**
 * @brief Unified connectivity policy interface for graph queries and edge edits.
 */
class IConnectivityGraph {
public:
    virtual ~IConnectivityGraph() = default;

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

    virtual bool add_directed_edge(common::CellIndex from, common::CellIndex to) = 0;
    virtual bool add_bidirectional_edge(common::CellIndex a, common::CellIndex b) = 0;
    virtual bool clear_edges() noexcept = 0;
    virtual bool has_directed_edge(common::CellIndex from, common::CellIndex to) const noexcept = 0;
};

/**
 * @brief Editable connectivity graph backed by an adjacency list.
 *
 * This policy allows arbitrary graph edges independent of geometric adjacency.
 */
class AdjacencyListGraph final : public IConnectivityGraph {
public:
    bool add_directed_edge(common::CellIndex from, common::CellIndex to) override;
    bool add_bidirectional_edge(common::CellIndex a, common::CellIndex b) override;
    bool clear_edges() noexcept override;
    bool has_directed_edge(common::CellIndex from, common::CellIndex to) const noexcept override;

    std::vector<common::CellIndex> neighbors(
        const WorldModel& world,
        common::CellIndex from
    ) const override;

private:
    std::unordered_map<common::CellIndex, std::vector<common::CellIndex>> adjacency_;
};

/**
 * @brief Editable connectivity graph backed by an adjacency matrix.
 *
 * This representation favors fast edge existence checks at the cost of memory.
 */
class AdjacencyMatrixGraph final : public IConnectivityGraph {
public:
    AdjacencyMatrixGraph() = default;
    explicit AdjacencyMatrixGraph(common::CellIndex initial_vertices);

    bool add_directed_edge(common::CellIndex from, common::CellIndex to) override;
    bool add_bidirectional_edge(common::CellIndex a, common::CellIndex b) override;
    bool clear_edges() noexcept override;
    bool has_directed_edge(common::CellIndex from, common::CellIndex to) const noexcept override;

    std::vector<common::CellIndex> neighbors(
        const WorldModel& world,
        common::CellIndex from
    ) const override;

private:
    void ensure_size(common::CellIndex vertex);

    std::vector<std::vector<std::uint8_t>> matrix_;
};

/// Alias exposing the library default connectivity policy name.
using DefaultConnectivityGraph = AdjacencyListGraph;

/// Shared ownership handle for connectivity policy objects.
using ConnectivityGraphPtr = std::shared_ptr<IConnectivityGraph>;

}  // namespace skymapf::world
