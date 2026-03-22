/**
 * @file scenario.cpp
 * @brief Implements ScenarioModel helper methods.
 */
#include "skymapf/scenario/model.hpp"

#include <optional>
#include <utility>

#include "skymapf/instance/model.hpp"
#include "skymapf/utils/default_naming.hpp"

namespace skymapf::scenario {

namespace {

instance::InstanceModel build_instance_from_task(
    const common::ScenarioId scenario_id,
    const world::WorldModel& world,
    const task::TaskModel& task_data,
    common::InstanceId instance_id,
    std::string instance_name
) {
    if (instance_name.empty()) {
        instance_name = utils::DefaultNaming::instance_name(scenario_id, task_data.id());
    }
    return instance::InstanceModel::create(
        instance_id,
        world,
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
            build_instance_from_task(id_, world_, task_data, instance_id++, {})
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
            id_,
            world_,
            task_data,
            instance_id,
            std::move(instance_name)
        );
    }
    return std::nullopt;
}

}  // namespace skymapf::scenario
