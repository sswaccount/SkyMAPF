/**
 * @file scenario.cpp
 * @brief Implements ScenarioModel helper methods.
 */
#include "skymapf/scenario/scenario.hpp"

#include "skymapf/generator/scenario_generator.hpp"

namespace skymapf::scenario {

std::vector<instance::InstanceModel> ScenarioModel::export_instances(
    common::InstanceId first_instance_id
) const {
    return generator::ScenarioGenerator::generate(
        *this,
        generator::InstanceGenerationOptions{first_instance_id}
    );
}

}  // namespace skymapf::scenario
