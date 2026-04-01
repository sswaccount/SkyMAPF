/**
* @file model.cpp
* @brief Implements world model state mutation and policy delegation.
*/

#include "skymapf/world/model.hpp"

#include <memory>
#include <stdexcept>
#include <utility>

#include "skymapf/common/index.hpp"
#include "skymapf/common/space.hpp"
#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/random_tool.hpp"
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
    return []() {
        return std::make_shared<DefaultConnectivityGraph>();
    };
}

}  // namespace

WorldModel::WorldModel(common::SpaceSpec space_spec)
    : WorldModel(
        utils::RandomTool::instance().next_id_value(),
        utils::DefaultNaming::next_world_name(space_spec),
        std::move(space_spec),
        {},
        {}
    ) {}

WorldModel::WorldModel(
    common::SpaceSpec space_spec,
    OccupancyFactory occupancy_factory,
    ConnectivityFactory connectivity_factory
)
    : WorldModel(
        utils::RandomTool::instance().next_id_value(),
        utils::DefaultNaming::next_world_name(space_spec),
        std::move(space_spec),
        std::move(occupancy_factory),
        std::move(connectivity_factory)
    ) {}

WorldModel::WorldModel(
    common::WorldId id,
    std::string name,
    common::SpaceSpec space_spec,
    OccupancyFactory occupancy_factory,
    ConnectivityFactory connectivity_factory
)
    : id_(id),
    name_(std::move(name)),
    space_spec_(std::move(space_spec)),
    occupancy_factory_(
        occupancy_factory ? std::move(occupancy_factory)
                            : build_default_occupancy_factory()
    ),
    connectivity_factory_(
        connectivity_factory ? std::move(connectivity_factory)
                            : build_default_connectivity_factory()
    ),
    occupancy_policy_(nullptr),
    connectivity_policy_(nullptr) {
        
    if (name_.empty()) {
        name_ = utils::DefaultNaming::next_world_name(space_spec_);
    }

    occupancy_policy_ = occupancy_factory_(common::cell_count(space_spec_));
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
    if (!occupancy_policy_ || !is_valid_index(index)) {
        return false;
    }
    return occupancy_policy_->is_walkable(index);
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

std::vector<common::CellIndex> WorldModel::walkable_cells() const {
    std::vector<common::CellIndex> cells;
    cells.reserve(static_cast<std::size_t>(cell_count()));
    for (common::CellIndex i = 0; i < cell_count(); ++i) {
        if (is_walkable(i)) {
            cells.push_back(i);
        }
    }
    return cells;
}

void WorldModel::set_walkable(common::CellIndex index, bool walkable) {
    if (!occupancy_policy_ || !is_valid_index(index)) {
        return;
    }
    occupancy_policy_->set_walkable(index, walkable);
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
    return connectivity_policy_->clear_edges();
}

bool WorldModel::add_directed_edge(common::CellIndex from, common::CellIndex to) {
    if (!connectivity_policy_ || !is_valid_index(from) || !is_valid_index(to)) {
        return false;
    }
    if (!is_walkable(from) || !is_walkable(to)) {
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
    if (!is_walkable(a) || !is_walkable(b)) {
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
    if (!is_walkable(from) || !is_walkable(to)) {
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

void to_json(nlohmann::ordered_json& j, const WorldModel& world_model)
{
    const common::GridShape3D export_shape{
        world_model.space_spec_.cols(),
        world_model.space_spec_.rows(),
        world_model.space_spec_.is_2d() ? 1 : world_model.space_spec_.layers()
    };

    const auto index_to_coord3_json = [&export_shape](common::CellIndex index) {
        const auto c3 = common::to_coord_3d(index, export_shape);
        return nlohmann::ordered_json::array({c3.x, c3.y, c3.z});
    };

    nlohmann::ordered_json obstacles = nlohmann::ordered_json::array();
    const auto total_cells = world_model.cell_count();
    for (common::CellIndex index = 0; index < total_cells; ++index) {
        if (!world_model.is_walkable(index)) {
            obstacles.push_back(index_to_coord3_json(index));
        }
    }

    nlohmann::ordered_json edges = nlohmann::ordered_json::array();
    for (common::CellIndex from = 0; from < total_cells; ++from) {
        for (common::CellIndex to = 0; to < total_cells; ++to) {
            if (!world_model.has_directed_edge(from, to)) {
                continue;
            }
            edges.push_back(nlohmann::ordered_json::array(
                {index_to_coord3_json(from), index_to_coord3_json(to)}
            ));
        }
    }

    j = nlohmann::ordered_json{
        {"id", world_model.id()},
        {"name", world_model.name()},
        {"space", world_model.space_spec()},
        {"obstacles", std::move(obstacles)},
        {"edges", std::move(edges)}
    };
    if (!world_model.info_.empty()) {
        j["info"] = world_model.info_;
    }
}

void from_json(const nlohmann::json& j, WorldModel& world_model)
{
    if (!j.contains("id")) {
        throw std::runtime_error("WorldModel JSON missing 'id'");
    }
    if (!j.contains("space")) {
        throw std::runtime_error("WorldModel JSON missing 'space'");
    }

    const auto parsed_id = j.at("id").get<common::WorldId>();
    const auto parsed_name = j.value("name", std::string{});
    const auto parsed_space = j.at("space").get<common::SpaceSpec>();

    if (!parsed_space.is_valid()) {
        throw std::runtime_error("WorldModel JSON contains invalid 'space'");
    }

    WorldModel parsed_world(parsed_id, parsed_name, parsed_space);
    parsed_world.info_ = j.value("info", std::string{});

    const common::GridShape3D shape_for_io{
        parsed_space.cols(),
        parsed_space.rows(),
        parsed_space.layers()
    };

    const auto coord3_to_index = [&shape_for_io](const nlohmann::json& coord_json) -> common::CellIndex {
        if (!coord_json.is_array() || coord_json.size() != 3) {
            throw std::runtime_error("WorldModel coordinate must be an array [x, y, z]");
        }

        const common::CellCoord3D coord{
            coord_json[0].get<std::int32_t>(),
            coord_json[1].get<std::int32_t>(),
            coord_json[2].get<std::int32_t>()
        };

        if (!common::is_in_bounds(coord, shape_for_io)) {
            throw std::runtime_error("WorldModel coordinate is out of bounds");
        }

        return common::to_index(coord, shape_for_io);
    };

    if (j.contains("obstacles")) {
        if (!j.at("obstacles").is_array()) {
            throw std::runtime_error("WorldModel field 'obstacles' must be an array");
        }
        for (const auto& obstacle_coord : j.at("obstacles")) {
            const auto index = coord3_to_index(obstacle_coord);
            parsed_world.set_walkable(index, false);
        }
    }

    if (j.contains("edges")) {
        if (!j.at("edges").is_array()) {
            throw std::runtime_error("WorldModel field 'edges' must be an array");
        }
        for (const auto& edge_json : j.at("edges")) {
            if (!edge_json.is_array() || edge_json.size() != 2) {
                throw std::runtime_error(
                    "WorldModel edge must be an array: [[x1, y1, z1], [x2, y2, z2]]"
                );
            }

            const auto from = coord3_to_index(edge_json[0]);
            const auto to = coord3_to_index(edge_json[1]);
            if (!parsed_world.add_directed_edge(from, to)) {
                throw std::runtime_error(
                    "WorldModel edge is invalid (out of bounds or touches obstacle)"
                );
            }
        }
    }

    world_model = std::move(parsed_world);
}

}  // namespace skymapf::world
