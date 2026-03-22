/**
 * @file scenario_generator.cpp
 * @brief Implements conversion from scenario models to executable instances.
 */
#include "skymapf/generator/scenario_generator.hpp"

namespace skymapf::generator {

std::vector<instance::InstanceModel> ScenarioGenerator::generate(
    const scenario::ScenarioModel& scenario_model,
    InstanceGenerationOptions options
) {
    std::vector<instance::InstanceModel> instances;
    instances.reserve(scenario_model.tasks().size());

    auto instance_id = options.first_instance_id;
    for (const auto& task_data : scenario_model.tasks()) {
        instances.push_back(instance::InstanceModel::create(
            instance_id++,
            scenario_model.world(),
            task_data,
            task_data.name()
        ));
    }
    return instances;
}

}  // namespace skymapf::generator
