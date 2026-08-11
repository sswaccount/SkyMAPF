#include "skymapf/algorithms/cbs/conflict_selector.hpp"

#include <algorithm>
#include <tuple>

#include "skymapf/search/conflict_detector.hpp"

namespace skymapf::algorithms::cbs {

namespace {

auto deterministic_key(const search::Conflict& conflict) {
    const auto first_agent = std::min(conflict.a1, conflict.a2);
    const auto second_agent = std::max(conflict.a1, conflict.a2);
    return std::make_tuple(
        conflict.time,
        conflict.kind == search::ConflictKind::Vertex ? 0 : 1,
        first_agent,
        second_agent,
        conflict.cell,
        conflict.a1_from,
        conflict.a1_to,
        conflict.a2_from,
        conflict.a2_to
    );
}

}  // namespace

bool ConflictSelector::select(
    const solution::Plan& plan,
    ConflictSelectionStrategy strategy,
    search::Conflict& selected
) {
    auto conflicts = search::ConflictDetector::detect_all(plan);
    if (conflicts.empty()) {
        return false;
    }
    if (strategy == ConflictSelectionStrategy::Earliest) {
        const auto iterator = std::min_element(
            conflicts.begin(),
            conflicts.end(),
            [](const auto& lhs, const auto& rhs) {
                return deterministic_key(lhs) < deterministic_key(rhs);
            }
        );
        selected = *iterator;
        return true;
    }
    selected = conflicts.front();
    return true;
}

}  // namespace skymapf::algorithms::cbs
