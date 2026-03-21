/**
 * @file connectivity.cpp
 * @brief Implements built-in connectivity policies for world traversal.
 */
#include "skymapf/world/connectivity.hpp"

#include <algorithm>
#include <cstddef>

#include "skymapf/common/index.hpp"
#include "skymapf/common/move.hpp"
#include "skymapf/world/world_model.hpp"

namespace skymapf::world {

std::vector<common::CellIndex> geometric_neighbors(
    const common::SpaceSpec& spec,
    common::CellIndex from
) {
    std::vector<common::CellIndex> result;
    if (spec.is_2d()) {
        const auto shape = spec.shape_2d();
        const auto c = common::to_coord_2d(from, shape);
        result.reserve(common::kMoveDelta4.size());

        for (const auto& delta : common::kMoveDelta4) {
            const common::CellCoord2D n{c.x + delta.x, c.y + delta.y};
            if (!common::is_in_bounds(n, shape)) {
                continue;
            }
            result.push_back(common::to_index(n, shape));
        }
        return result;
    }

    const auto shape = spec.shape_3d();
    const auto c = common::to_coord_3d(from, shape);
    result.reserve(common::kMoveDelta6.size());

    for (const auto& delta : common::kMoveDelta6) {
        const common::CellCoord3D n{c.x + delta.x, c.y + delta.y, c.z + delta.z};
        if (!common::is_in_bounds(n, shape)) {
            continue;
        }
        result.push_back(common::to_index(n, shape));
    }
    return result;
}

bool AdjacencyListGraph::add_directed_edge(common::CellIndex from, common::CellIndex to) {
    adjacency_[from].push_back(to);
    return true;
}

bool AdjacencyListGraph::add_bidirectional_edge(common::CellIndex a, common::CellIndex b) {
    add_directed_edge(a, b);
    add_directed_edge(b, a);
    return true;
}

bool AdjacencyListGraph::clear_edge() noexcept {
    adjacency_.clear();
    return true;
}

bool AdjacencyListGraph::has_directed_edge(
    common::CellIndex from,
    common::CellIndex to
) const noexcept {
    const auto it = adjacency_.find(from);
    if (it == adjacency_.end()) {
        return false;
    }
    const auto& out = it->second;
    return std::find(out.begin(), out.end(), to) != out.end();
}

std::vector<common::CellIndex> AdjacencyListGraph::neighbors(
    const WorldModel& world,
    common::CellIndex from
) const {
    if (!world.is_valid_index(from) || !world.is_walkable(from)) {
        return {};
    }

    const auto it = adjacency_.find(from);
    if (it == adjacency_.end()) {
        return {};
    }

    std::vector<common::CellIndex> result;
    result.reserve(it->second.size());
    for (const auto to : it->second) {
        if (!world.is_valid_index(to)) {
            continue;
        }
        if (!world.is_walkable(to)) {
            continue;
        }
        result.push_back(to);
    }
    return result;
}

AdjacencyMatrixGraph::AdjacencyMatrixGraph(common::CellIndex initial_vertices) {
    if (initial_vertices == 0) {
        return;
    }
    ensure_size(initial_vertices - 1);
}

void AdjacencyMatrixGraph::ensure_size(common::CellIndex vertex) {
    const auto required = static_cast<std::size_t>(vertex + 1);
    if (required <= matrix_.size()) {
        return;
    }
    for (auto& row : matrix_) {
        row.resize(required, 0u);
    }
    matrix_.resize(required, std::vector<std::uint8_t>(required, 0u));
}

bool AdjacencyMatrixGraph::add_directed_edge(common::CellIndex from, common::CellIndex to) {
    ensure_size(std::max(from, to));
    matrix_[static_cast<std::size_t>(from)][static_cast<std::size_t>(to)] = 1u;
    return true;
}

bool AdjacencyMatrixGraph::add_bidirectional_edge(common::CellIndex a, common::CellIndex b) {
    add_directed_edge(a, b);
    add_directed_edge(b, a);
    return true;
}

bool AdjacencyMatrixGraph::clear_edge() noexcept {
    for (auto& row : matrix_) {
        std::fill(row.begin(), row.end(), 0u);
    }
    return true;
}

bool AdjacencyMatrixGraph::has_directed_edge(
    common::CellIndex from,
    common::CellIndex to
) const noexcept {
    const auto from_i = static_cast<std::size_t>(from);
    const auto to_i = static_cast<std::size_t>(to);
    if (from_i >= matrix_.size() || to_i >= matrix_.size()) {
        return false;
    }
    return matrix_[from_i][to_i] != 0u;
}

std::vector<common::CellIndex> matrix_outgoing_edges(
    const std::vector<std::vector<std::uint8_t>>& matrix,
    common::CellIndex from
) {
    const auto from_i = static_cast<std::size_t>(from);
    if (from_i >= matrix.size()) {
        return {};
    }
    std::vector<common::CellIndex> out;
    const auto& row = matrix[from_i];
    for (std::size_t to = 0; to < row.size(); ++to) {
        if (row[to] != 0u) {
            out.push_back(static_cast<common::CellIndex>(to));
        }
    }
    return out;
}

std::vector<common::CellIndex> AdjacencyMatrixGraph::neighbors(
    const WorldModel& world,
    common::CellIndex from
) const {
    if (!world.is_valid_index(from) || !world.is_walkable(from)) {
        return {};
    }
    std::vector<common::CellIndex> out;
    for (const auto to : matrix_outgoing_edges(matrix_, from)) {
        if (!world.is_valid_index(to) || !world.is_walkable(to)) {
            continue;
        }
        out.push_back(to);
    }
    return out;
}

}  // namespace skymapf::world
