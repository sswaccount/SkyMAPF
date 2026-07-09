#include "skymapf/search/conflict_detector.hpp"

#include <algorithm>

namespace skymapf::search {

std::vector<Conflict> ConflictDetector::detect_all(const solution::Plan& plan) {
    std::vector<Conflict> conflicts;
    for (std::size_t i = 0; i < plan.agent_paths.size(); ++i) {
        for (std::size_t j = i + 1; j < plan.agent_paths.size(); ++j) {
            const auto& a = plan.agent_paths[i];
            const auto& b = plan.agent_paths[j];
            const auto horizon = std::max(a.end_time(), b.end_time());
            for (common::TimeStep t = 0; t <= horizon; ++t) {
                const auto a_loc = a.location_at(t);
                const auto b_loc = b.location_at(t);
                if (a_loc == b_loc) {
                    Conflict conflict;
                    conflict.a1 = a.agent_id;
                    conflict.a2 = b.agent_id;
                    conflict.kind = ConflictKind::Vertex;
                    conflict.time = t;
                    conflict.cell = a_loc;
                    conflicts.push_back(conflict);
                }
                if (t == 0) {
                    continue;
                }
                const auto a_prev = a.location_at(static_cast<common::TimeStep>(t - 1));
                const auto b_prev = b.location_at(static_cast<common::TimeStep>(t - 1));
                if (a_prev == b_loc && b_prev == a_loc) {
                    Conflict conflict;
                    conflict.a1 = a.agent_id;
                    conflict.a2 = b.agent_id;
                    conflict.kind = ConflictKind::Edge;
                    conflict.time = t;
                    conflict.a1_from = a_prev;
                    conflict.a1_to = a_loc;
                    conflict.a2_from = b_prev;
                    conflict.a2_to = b_loc;
                    conflicts.push_back(conflict);
                }
            }
        }
    }
    return conflicts;
}

bool ConflictDetector::first_conflict(
    const solution::Plan& plan,
    Conflict& conflict
) {
    const auto conflicts = detect_all(plan);
    if (conflicts.empty()) {
        return false;
    }
    conflict = conflicts.front();
    return true;
}

}  // namespace skymapf::search
