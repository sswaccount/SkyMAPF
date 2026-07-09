#include "skymapf/solution/metrics.hpp"

#include <algorithm>

namespace skymapf::solution {

SolutionMetrics compute_metrics(const Plan& plan) {
    SolutionMetrics metrics;
    for (const auto& path : plan.agent_paths) {
        if (!path.cells.empty()) {
            metrics.sum_of_costs += path.cells.size() - 1;
            metrics.makespan = std::max(metrics.makespan, path.end_time());
        }
    }
    return metrics;
}

}  // namespace skymapf::solution
