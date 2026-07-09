/**
 * @file space_time_astar.hpp
 * @brief Declares a basic space-time A* single-agent planner.
 */
#pragma once

#include "i_single_agent_planner.hpp"

namespace skymapf::algorithms::low_level {

class SpaceTimeAStar final : public ISingleAgentPlanner {
public:
    LowLevelSearchResult find_path(
        const instance::InstanceModel& instance,
        common::AgentId agent_id,
        const search::ConstraintTable& constraints,
        const search::ReservationTable& reservations,
        const LowLevelSearchOptions& options
    ) override;
};

}  // namespace skymapf::algorithms::low_level
