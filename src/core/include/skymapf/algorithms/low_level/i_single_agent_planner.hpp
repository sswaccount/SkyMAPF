/**
 * @file i_single_agent_planner.hpp
 * @brief Declares a reusable single-agent planner interface.
 */
#pragma once

#include "low_level_options.hpp"
#include "low_level_result.hpp"
#include "skymapf/instance/model.hpp"
#include "skymapf/search/constraint_table.hpp"
#include "skymapf/search/reservation_table.hpp"

namespace skymapf::algorithms::low_level {

class ISingleAgentPlanner {
public:
    virtual ~ISingleAgentPlanner() = default;

    virtual LowLevelSearchResult find_path(
        const instance::InstanceModel& instance,
        common::AgentId agent_id,
        const search::ConstraintTable& constraints,
        const search::ReservationTable& reservations,
        const LowLevelSearchOptions& options
    ) = 0;
};

}  // namespace skymapf::algorithms::low_level
