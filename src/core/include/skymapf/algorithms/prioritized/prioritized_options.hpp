/**
 * @file prioritized_options.hpp
 * @brief Defines prioritized planning solver options.
 */
#pragma once

#include <vector>

#include "skymapf/common/ids.hpp"
#include "skymapf/algorithms/low_level/low_level_options.hpp"

namespace skymapf::algorithms::prioritized {

struct PrioritizedPlanningOptions {
    std::vector<common::AgentId> priority_order;
    bool reserve_goals_forever{true};
    low_level::LowLevelSearchOptions low_level;
};

}  // namespace skymapf::algorithms::prioritized
