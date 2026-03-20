#pragma once

#include <vector>

#include "../common/coord.hpp"
#include "../common/ids.hpp"
#include "../common/space.hpp"
#include "connectivity.hpp"
#include "occupancy.hpp"

namespace skymapf::world {

class WorldModel {
public:
    WorldModel();
    explicit WorldModel(common::SpaceSpec space_spec, bool default_walkable = true);

    const common::SpaceSpec& space_spec() const noexcept { return space_spec_; }
    void set_space_spec(common::SpaceSpec space_spec);

    common::CellIndex cell_count() const noexcept;
    bool is_valid_index(common::CellIndex index) const noexcept;

    bool is_walkable(common::CellIndex index) const noexcept;
    bool is_walkable(const common::CellCoord2D& coord) const noexcept;
    bool is_walkable(const common::CellCoord3D& coord) const noexcept;

    void set_walkable(common::CellIndex index, bool walkable);
    void set_walkable(const common::CellCoord2D& coord, bool walkable);
    void set_walkable(const common::CellCoord3D& coord, bool walkable);

    std::vector<common::CellIndex> neighbors(common::CellIndex from) const;

    void set_occupancy_map(OccupancyMapPtr occupancy);
    const OccupancyMapPtr& occupancy_map() const noexcept { return occupancy_; }

    void set_connectivity_policy(ConnectivityPolicyPtr policy);
    const ConnectivityPolicyPtr& connectivity_policy() const noexcept { return connectivity_policy_; }

private:
    common::SpaceSpec space_spec_;
    OccupancyMapPtr occupancy_;
    ConnectivityPolicyPtr connectivity_policy_;
};

}  // namespace skymapf::world
