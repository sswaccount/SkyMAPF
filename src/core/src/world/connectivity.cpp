#include "skymapf/world/connectivity.hpp"

#include "skymapf/common/index.hpp"
#include "skymapf/common/move.hpp"
#include "skymapf/world/world_model.hpp"

namespace skymapf::world {

std::vector<common::CellIndex> GridAdjacencyPolicy::neighbors(
    const WorldModel& world,
    common::CellIndex from
) const {
    if (!world.is_valid_index(from) || !world.is_walkable(from)) {
        return {};
    }

    std::vector<common::CellIndex> result;
    const auto& spec = world.space_spec();

    if (spec.is_2d()) {
        const auto shape = spec.shape_2d();
        const auto c = common::to_coord_2d(from, shape);
        result.reserve(common::kMoveDelta4.size());

        for (const auto& delta : common::kMoveDelta4) {
            const common::CellCoord2D n{c.x + delta.x, c.y + delta.y};
            if (!common::is_in_bounds(n, shape)) {
                continue;
            }
            const auto n_idx = common::to_index(n, shape);
            if (world.is_walkable(n_idx)) {
                result.push_back(n_idx);
            }
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
        const auto n_idx = common::to_index(n, shape);
        if (world.is_walkable(n_idx)) {
            result.push_back(n_idx);
        }
    }
    return result;
}

void ExplicitGraphConnectivityPolicy::add_directed_edge(common::CellIndex from, common::CellIndex to) {
    adjacency_[from].push_back(to);
}

void ExplicitGraphConnectivityPolicy::add_bidirectional_edge(common::CellIndex a, common::CellIndex b) {
    add_directed_edge(a, b);
    add_directed_edge(b, a);
}

void ExplicitGraphConnectivityPolicy::clear() noexcept {
    adjacency_.clear();
}

std::vector<common::CellIndex> ExplicitGraphConnectivityPolicy::neighbors(
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

}  // namespace skymapf::world
