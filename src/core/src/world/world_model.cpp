#include "skymapf/world/world_model.hpp"

#include <utility>

#include "skymapf/common/index.hpp"
#include "skymapf/common/space.hpp"

namespace skymapf::world {

namespace {

common::CellIndex calc_cell_count(const common::SpaceSpec& spec) noexcept {
    if (!spec.is_valid()) {
        return 0;
    }
    if (spec.is_2d()) {
        const auto shape = spec.shape_2d();
        return static_cast<common::CellIndex>(shape.cols) * static_cast<common::CellIndex>(shape.rows);
    }
    const auto shape = spec.shape_3d();
    return static_cast<common::CellIndex>(shape.cols)
         * static_cast<common::CellIndex>(shape.rows)
         * static_cast<common::CellIndex>(shape.layers);
}

}  // namespace

WorldModel::WorldModel()
    : WorldModel(common::SpaceSpec::make_2d(1, 1), true) {}

WorldModel::WorldModel(common::SpaceSpec space_spec, bool default_walkable)
    : space_spec_(std::move(space_spec)),
      occupancy_(std::make_shared<DenseOccupancyMap>(calc_cell_count(space_spec_), default_walkable)),
      connectivity_policy_(std::make_shared<GridAdjacencyPolicy>()) {}

void WorldModel::set_space_spec(common::SpaceSpec space_spec) {
    space_spec_ = std::move(space_spec);
    occupancy_ = std::make_shared<DenseOccupancyMap>(calc_cell_count(space_spec_), true);
}

common::CellIndex WorldModel::cell_count() const noexcept {
    return calc_cell_count(space_spec_);
}

bool WorldModel::is_valid_index(common::CellIndex index) const noexcept {
    return index < cell_count();
}

bool WorldModel::is_walkable(common::CellIndex index) const noexcept {
    if (!occupancy_) {
        return false;
    }
    return occupancy_->is_walkable(index);
}

bool WorldModel::is_walkable(const common::CellCoord2D& coord) const noexcept {
    if (!space_spec_.is_2d()) {
        return false;
    }
    const auto shape = space_spec_.shape_2d();
    if (!common::is_in_bounds(coord, shape)) {
        return false;
    }
    return is_walkable(common::to_index(coord, shape));
}

bool WorldModel::is_walkable(const common::CellCoord3D& coord) const noexcept {
    if (!space_spec_.is_3d()) {
        return false;
    }
    const auto shape = space_spec_.shape_3d();
    if (!common::is_in_bounds(coord, shape)) {
        return false;
    }
    return is_walkable(common::to_index(coord, shape));
}

void WorldModel::set_walkable(common::CellIndex index, bool walkable) {
    if (!occupancy_) {
        return;
    }
    occupancy_->set_walkable(index, walkable);
}

void WorldModel::set_walkable(const common::CellCoord2D& coord, bool walkable) {
    if (!space_spec_.is_2d()) {
        return;
    }
    const auto shape = space_spec_.shape_2d();
    if (!common::is_in_bounds(coord, shape)) {
        return;
    }
    set_walkable(common::to_index(coord, shape), walkable);
}

void WorldModel::set_walkable(const common::CellCoord3D& coord, bool walkable) {
    if (!space_spec_.is_3d()) {
        return;
    }
    const auto shape = space_spec_.shape_3d();
    if (!common::is_in_bounds(coord, shape)) {
        return;
    }
    set_walkable(common::to_index(coord, shape), walkable);
}

std::vector<common::CellIndex> WorldModel::neighbors(common::CellIndex from) const {
    if (!connectivity_policy_ || !is_valid_index(from)) {
        return {};
    }
    return connectivity_policy_->neighbors(*this, from);
}

void WorldModel::set_occupancy_map(OccupancyMapPtr occupancy) {
    if (!occupancy || occupancy->cell_count() != cell_count()) {
        return;
    }
    occupancy_ = std::move(occupancy);
}

void WorldModel::set_connectivity_policy(ConnectivityPolicyPtr policy) {
    if (!policy) {
        return;
    }
    connectivity_policy_ = std::move(policy);
}

}  // namespace skymapf::world
