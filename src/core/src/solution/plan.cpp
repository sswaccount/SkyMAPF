#include "skymapf/solution/plan.hpp"

namespace skymapf::solution {

const AgentPath* Plan::find_path(common::AgentId agent_id) const noexcept {
    for (const auto& path : agent_paths) {
        if (path.agent_id == agent_id) {
            return &path;
        }
    }
    return nullptr;
}

AgentPath* Plan::find_path(common::AgentId agent_id) noexcept {
    for (auto& path : agent_paths) {
        if (path.agent_id == agent_id) {
            return &path;
        }
    }
    return nullptr;
}

}  // namespace skymapf::solution
