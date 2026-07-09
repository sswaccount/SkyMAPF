/**
 * @file path.hpp
 * @brief Defines path models shared by solver implementations.
 */
#pragma once

#include <cstddef>
#include <vector>

#include "skymapf/common/ids.hpp"

namespace skymapf::solution {

/// One agent location at one discrete timestep.
struct TimedCell {
    common::TimeStep time{0};
    common::CellIndex cell{0};
};

/// Ordered path assigned to one agent.
struct AgentPath {
    common::AgentId agent_id{0};
    common::TimeStep start_time{0};
    std::vector<common::CellIndex> cells;

    bool empty() const noexcept { return cells.empty(); }
    std::size_t size() const noexcept { return cells.size(); }
    common::CellIndex front() const noexcept { return cells.front(); }
    common::CellIndex back() const noexcept { return cells.back(); }

    common::TimeStep end_time() const noexcept;
    common::CellIndex location_at(common::TimeStep time) const noexcept;
};

}  // namespace skymapf::solution
