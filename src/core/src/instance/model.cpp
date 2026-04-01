/**
 * @file model.cpp
 * @brief Implements InstanceModel conversion helpers.
 */
#include "skymapf/instance/model.hpp"

#include <algorithm>
#include <unordered_set>
#include <vector>

#include "skymapf/scenario/model.hpp"
#include "skymapf/utils/default_naming.hpp"

namespace skymapf::instance {

const task::AgentTaskEntry* InstanceModel::find_agent_task(common::AgentId agent_id) const noexcept {
    return task_.find_agent_entry(agent_id);
}

const InstanceModel::ResolvedAgentTask* InstanceModel::find_assignment(
    common::AgentId agent_id
) const noexcept {
    const auto it = std::find_if(
        resolved_bindings_.begin(),
        resolved_bindings_.end(),
        [agent_id](const ResolvedAgentTask& assignment) { return assignment.agent_id == agent_id; }
    );
    if (it == resolved_bindings_.end()) {
        return nullptr;
    }
    return &(*it);
}

bool InstanceModel::has_consistent_assignments() const noexcept {
    std::unordered_set<common::AgentId> seen;
    seen.reserve(task_.agent_entries().size());
    for (const auto& entry : task_.agent_entries()) {
        if (!seen.insert(entry.agent_id).second) {
            return false;
        }
    }
    return true;
}

void InstanceModel::rebuild_resolved_bindings() {
    resolved_bindings_.clear();
    resolved_bindings_.reserve(task_.agent_entries().size());

    for (std::size_t task_index = 0; task_index < task_.agent_entries().size(); ++task_index) {
        const auto& entry = task_.agent_entries()[task_index];
        resolved_bindings_.push_back(ResolvedAgentTask{
            entry.agent_id,
            task_index
        });
    }
}

scenario::ScenarioModel InstanceModel::to_scenario(
    common::ScenarioId scenario_id,
    std::string scenario_name
) const {
    if (scenario_name.empty()) {
        scenario_name = utils::DefaultNaming::next_scenario_name();
    }
    std::vector<task::TaskModel> tasks;
    tasks.push_back(task_);
    return scenario::ScenarioModel::create(
        scenario_id,
        std::move(scenario_name),
        world_,
        std::move(tasks)
    );
}

}  // namespace skymapf::instance
