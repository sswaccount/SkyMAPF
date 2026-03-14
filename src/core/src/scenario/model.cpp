/**
 * @file scenario.cpp
 * @brief Implements ScenarioModel helper methods.
 */
#include "skymapf/scenario/model.hpp"

#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "skymapf/instance/model.hpp"
#include "skymapf/utils/checksum.hpp"
#include "skymapf/utils/default_naming.hpp"

namespace skymapf::scenario {

namespace {

instance::InstanceModel build_instance_from_task(
    const world::WorldModel& world,
    const task::TaskModel& task_data,
    common::InstanceId instance_id,
    std::string instance_name
) {
    if (instance_name.empty()) {
        instance_name = utils::DefaultNaming::next_instance_name();
    }
    return instance::InstanceModel::create(
        instance_id,
        world,
        task_data,
        std::move(instance_name)
    );
}

}  // namespace

void to_json(nlohmann::ordered_json& j, const ScenarioMeta& meta) {
    j = nlohmann::ordered_json{
        {"scenario_id", meta.scenario_id},
        {"name", meta.name},
        {"info", meta.info},
        {"world_id", meta.world_id},
        {"task_ids", meta.task_ids},
        {"checksum", meta.checksum}
    };
}

void from_json(const nlohmann::json& j, ScenarioMeta& meta) {
    if (!j.contains("scenario_id")) {
        throw std::runtime_error("ScenarioMeta JSON missing 'scenario_id'.");
    }
    if (!j.contains("world_id")) {
        throw std::runtime_error("ScenarioMeta JSON missing 'world_id'.");
    }
    if (!j.contains("task_ids")) {
        throw std::runtime_error("ScenarioMeta JSON missing 'task_ids'.");
    }

    meta.scenario_id = j.at("scenario_id").get<common::ScenarioId>();
    meta.name = j.value("name", std::string{});
    meta.info = j.value("info", std::string{});
    meta.world_id = j.at("world_id").get<common::WorldId>();
    meta.task_ids = j.at("task_ids").get<std::vector<common::TaskId>>();
    meta.checksum = j.value("checksum", std::string{});
}

ScenarioMeta ScenarioModel::meta() const {
    ScenarioMeta result;
    result.scenario_id = id_;
    result.name = name_;
    result.info = info_;
    result.world_id = world_.id();
    result.task_ids.reserve(tasks_.size());
    for (const auto& one_task : tasks_) {
        result.task_ids.push_back(one_task.id());
    }
    result.checksum = utils::fnv1a64_hex(utils::build_scenario_meta_checksum_payload(
        result.scenario_id,
        result.name,
        result.info,
        random_seed_,
        result.world_id,
        result.task_ids
    ));
    return result;
}

std::vector<instance::InstanceModel> ScenarioModel::export_instances(
    common::InstanceId first_instance_id
) const {
    std::vector<instance::InstanceModel> instances;
    instances.reserve(tasks_.size());
    auto instance_id = first_instance_id;
    for (const auto& task_data : tasks_) {
        instances.push_back(
            build_instance_from_task(world_, task_data, instance_id++, {})
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
            task_data,
            instance_id,
            std::move(instance_name)
        );
    }
    return std::nullopt;
}

}  // namespace skymapf::scenario
