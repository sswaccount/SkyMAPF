/**
 * @file world_model.cpp
 * @brief Implements world model state mutation and policy delegation.
 */
#include "skymapf/world/world_model.hpp"

#include <memory>
#include <utility>

#include "skymapf/common/index.hpp"
#include "skymapf/common/space.hpp"
#include "skymapf/world/connectivity.hpp"
#include "skymapf/world/occupancy.hpp"

namespace skymapf::world {

namespace {

WorldModel::OccupancyFactory build_default_occupancy_factory() {
    return [](common::CellIndex cell_count) {
        return std::make_shared<DefaultOccupancyStore>(cell_count);
    };
}

WorldModel::ConnectivityFactory build_default_connectivity_factory() {
    return []() { return std::make_shared<DefaultConnectivityGraph>(); };
}

}  // namespace

WorldModel::WorldModel()
    : WorldModel(
        common::SpaceSpec::make_2d(1, 1),
        build_default_occupancy_factory(),
        build_default_connectivity_factory()
    ) {}

WorldModel::WorldModel(
    common::SpaceSpec space_spec,
    OccupancyFactory occupancy_factory,
    ConnectivityFactory connectivity_factory
)
    : space_spec_(std::move(space_spec)),
      occupancy_factory_(occupancy_factory ? std::move(occupancy_factory) : build_default_occupancy_factory()),
      connectivity_factory_(
          connectivity_factory ? std::move(connectivity_factory) : build_default_connectivity_factory()
      ),
      occupancy_(nullptr),
      connectivity_policy_(nullptr) {
    occupancy_ = occupancy_factory_(common::cell_count(space_spec_));
    connectivity_policy_ = connectivity_factory_();
}

common::CellIndex WorldModel::cell_count() const noexcept {
    return common::cell_count(space_spec_);
}

bool WorldModel::is_valid_index(common::CellIndex index) const noexcept {
    return index < cell_count();
}

bool WorldModel::is_valid_coord(const common::CellCoord2D& coord) const noexcept {
    if (!space_spec_.is_2d()) {
        return false;
    }
    return common::is_in_bounds(coord, space_spec_.shape_2d());
}

bool WorldModel::is_valid_coord(const common::CellCoord3D& coord) const noexcept {
    if (!space_spec_.is_3d()) {
        return false;
    }
    return common::is_in_bounds(coord, space_spec_.shape_3d());
}

bool WorldModel::is_walkable(common::CellIndex index) const noexcept {
    if (!occupancy_) {
        return false;
    }
    return occupancy_->is_walkable(index);
}

bool WorldModel::is_walkable(const common::CellCoord2D& coord) const noexcept {
    if (!is_valid_coord(coord)) {
        return false;
    }
    return is_walkable(common::to_index(coord, space_spec_.shape_2d()));
}

bool WorldModel::is_walkable(const common::CellCoord3D& coord) const noexcept {
    if (!is_valid_coord(coord)) {
        return false;
    }
    return is_walkable(common::to_index(coord, space_spec_.shape_3d()));
}

void WorldModel::set_walkable(common::CellIndex index, bool walkable) {
    if (!occupancy_) {
        return;
    }
    occupancy_->set_walkable(index, walkable);
}

void WorldModel::set_walkable(const common::CellCoord2D& coord, bool walkable) {
    if (!is_valid_coord(coord)) {
        return;
    }
    set_walkable(common::to_index(coord, space_spec_.shape_2d()), walkable);
}

void WorldModel::set_walkable(const common::CellCoord3D& coord, bool walkable) {
    if (!is_valid_coord(coord)) {
        return;
    }
    set_walkable(common::to_index(coord, space_spec_.shape_3d()), walkable);
}

std::vector<common::CellIndex> WorldModel::neighbors(common::CellIndex from) const {
    if (!connectivity_policy_ || !is_valid_index(from)) {
        return {};
    }
    return connectivity_policy_->neighbors(*this, from);
}

std::vector<common::CellIndex> WorldModel::neighbors(const common::CellCoord2D& from) const {
    if (!is_valid_coord(from)) {
        return {};
    }
    return neighbors(common::to_index(from, space_spec_.shape_2d()));
}

std::vector<common::CellIndex> WorldModel::neighbors(const common::CellCoord3D& from) const {
    if (!is_valid_coord(from)) {
        return {};
    }
    return neighbors(common::to_index(from, space_spec_.shape_3d()));
}

bool WorldModel::clear_edges() {
    if (!connectivity_policy_) {
        return false;
    }
    return connectivity_policy_->clear_edge();
}

bool WorldModel::add_directed_edge(common::CellIndex from, common::CellIndex to) {
    if (!connectivity_policy_ || !is_valid_index(from) || !is_valid_index(to)) {
        return false;
    }
    return connectivity_policy_->add_directed_edge(from, to);
}

bool WorldModel::add_directed_edge(const common::CellCoord2D& from, const common::CellCoord2D& to) {
    if (!is_valid_coord(from) || !is_valid_coord(to)) {
        return false;
    }
    return add_directed_edge(
        common::to_index(from, space_spec_.shape_2d()),
        common::to_index(to, space_spec_.shape_2d())
    );
}

bool WorldModel::add_directed_edge(const common::CellCoord3D& from, const common::CellCoord3D& to) {
    if (!is_valid_coord(from) || !is_valid_coord(to)) {
        return false;
    }
    return add_directed_edge(
        common::to_index(from, space_spec_.shape_3d()),
        common::to_index(to, space_spec_.shape_3d())
    );
}

bool WorldModel::add_bidirectional_edge(common::CellIndex a, common::CellIndex b) {
    if (!connectivity_policy_ || !is_valid_index(a) || !is_valid_index(b)) {
        return false;
    }
    return connectivity_policy_->add_bidirectional_edge(a, b);
}

bool WorldModel::add_bidirectional_edge(const common::CellCoord2D& a, const common::CellCoord2D& b) {
    if (!is_valid_coord(a) || !is_valid_coord(b)) {
        return false;
    }
    return add_bidirectional_edge(
        common::to_index(a, space_spec_.shape_2d()),
        common::to_index(b, space_spec_.shape_2d())
    );
}

bool WorldModel::add_bidirectional_edge(const common::CellCoord3D& a, const common::CellCoord3D& b) {
    if (!is_valid_coord(a) || !is_valid_coord(b)) {
        return false;
    }
    return add_bidirectional_edge(
        common::to_index(a, space_spec_.shape_3d()),
        common::to_index(b, space_spec_.shape_3d())
    );
}

bool WorldModel::has_directed_edge(common::CellIndex from, common::CellIndex to) const noexcept {
    if (!connectivity_policy_ || !is_valid_index(from) || !is_valid_index(to)) {
        return false;
    }
    return connectivity_policy_->has_directed_edge(from, to);
}

bool WorldModel::has_directed_edge(
    const common::CellCoord2D& from,
    const common::CellCoord2D& to
) const noexcept {
    if (!is_valid_coord(from) || !is_valid_coord(to)) {
        return false;
    }
    return has_directed_edge(
        common::to_index(from, space_spec_.shape_2d()),
        common::to_index(to, space_spec_.shape_2d())
    );
}

bool WorldModel::has_directed_edge(
    const common::CellCoord3D& from,
    const common::CellCoord3D& to
) const noexcept {
    if (!is_valid_coord(from) || !is_valid_coord(to)) {
        return false;
    }
    return has_directed_edge(
        common::to_index(from, space_spec_.shape_3d()),
        common::to_index(to, space_spec_.shape_3d())
    );
}

}  // namespace skymapf::world
