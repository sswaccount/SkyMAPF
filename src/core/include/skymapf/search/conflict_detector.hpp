/**
 * @file conflict_detector.hpp
 * @brief Declares conflict detection over multi-agent plans.
 */
#pragma once

#include <vector>

#include "conflict.hpp"
#include "skymapf/solution/plan.hpp"

namespace skymapf::search {

class ConflictDetector {
public:
    static std::vector<Conflict> detect_all(const solution::Plan& plan);
    static bool first_conflict(const solution::Plan& plan, Conflict& conflict);
};

}  // namespace skymapf::search
