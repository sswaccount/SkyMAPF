/**
 * @file plan.hpp
 * @brief Defines multi-agent plans produced by solvers.
 */
#pragma once

#include <optional>
#include <vector>

#include "path.hpp"
#include "skymapf/common/ids.hpp"

namespace skymapf::solution {

/// Complete multi-agent plan represented as one path per planned agent.
struct Plan {
    std::vector<AgentPath> agent_paths;

    bool empty() const noexcept { return agent_paths.empty(); }
    const AgentPath* find_path(common::AgentId agent_id) const noexcept;
    AgentPath* find_path(common::AgentId agent_id) noexcept;
};

}  // namespace skymapf::solution
