/**
 * @file scenario.cpp
 * @brief Implements ScenarioModel helper methods.
 */
#include "skymapf/scenario/model.hpp"

#include <optional>
#include <utility>
#include <vector>

#include "skymapf/agent/model.hpp"
#include "skymapf/instance/model.hpp"
#include "skymapf/utils/default_naming.hpp"

namespace skymapf::scenario {

namespace {

std::vector<agent::AgentModel> derive_agents_from_task(const task::TaskModel& task_data) {
    std::vector<agent::AgentModel> derived_agents;
    derived_agents.reserve(task_data.agent_entries().size());
    for (const auto& entry : task_data.agent_entries()) {
        derived_agents.emplace_back(entry.agent_id, utils::DefaultNaming::next_agent_name());
    }
    return derived_agents;
}

instance::InstanceModel build_instance_from_task(
    const world::WorldModel& world,
    const std::vector<agent::AgentModel>& scenario_agents,
    const task::TaskModel& task_data,
    common::InstanceId instance_id,
    std::string instance_name
) {
    if (instance_name.empty()) {
        instance_name = utils::DefaultNaming::next_instance_name();
    }
    auto bound_agents = scenario_agents.empty() ? derive_agents_from_task(task_data) : scenario_agents;
    return instance::InstanceModel::create(
        instance_id,
        world,
        std::move(bound_agents),
        task_data,
        std::move(instance_name)
    );
}

}  // namespace

std::vector<instance::InstanceModel> ScenarioModel::export_instances(
    common::InstanceId first_instance_id
) const {
    std::vector<instance::InstanceModel> instances;
    instances.reserve(tasks_.size());
    auto instance_id = first_instance_id;
    for (const auto& task_data : tasks_) {
        instances.push_back(
            build_instance_from_task(world_, agents_, task_data, instance_id++, {})
        );
    }
    return instances;
}

std::optional<instance::InstanceModel> ScenarioModel::export_instance_by_task_id(
    common::TaskId task_id,
    common::InstanceId instance_id,
    std::string instance_name
) const {
    for (const auto& task_data : tasks_) {
        if (task_data.id() != task_id) {
            continue;
        }
        return build_instance_from_task(
            world_,
            agents_,
            task_data,
            instance_id,
            std::move(instance_name)
        );
    }
    return std::nullopt;
}

}  // namespace skymapf::scenario
