/**
 * @file low_level_result.hpp
 * @brief Defines low-level single-agent search outputs.
 */
#pragma once

#include <string>

#include "skymapf/search/search_stats.hpp"
#include "skymapf/solution/path.hpp"

namespace skymapf::algorithms::low_level {

enum class LowLevelStatus {
    Success,
    Infeasible,
    NodeLimit,
    Error,
};

struct LowLevelSearchResult {
    LowLevelStatus status{LowLevelStatus::Error};
    solution::AgentPath path;
    search::SearchStats stats;
    std::string message;
};

}  // namespace skymapf::algorithms::low_level
