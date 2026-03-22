/**
 * @file model.cpp
 * @brief Implements InstanceModel conversion helpers.
 */
#include "skymapf/instance/model.hpp"

#include <vector>

#include "skymapf/scenario/model.hpp"
#include "skymapf/utils/default_naming.hpp"

namespace skymapf::instance {

scenario::ScenarioModel InstanceModel::to_scenario(
    common::ScenarioId scenario_id,
    std::string scenario_name
) const {
    if (scenario_name.empty()) {
        scenario_name = utils::DefaultNaming::scenario_name(id_, task_.id());
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
