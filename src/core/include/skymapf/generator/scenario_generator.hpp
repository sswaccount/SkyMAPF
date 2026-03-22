/**
 * @file scenario_generator.hpp
 * @brief Defines conversion helpers from scenario models to executable instances.
 */
#pragma once

#include <vector>

#include "../common/ids.hpp"
#include "../instance/instance.hpp"
#include "../scenario/scenario.hpp"

namespace skymapf::generator {

/// Backward-compatible alias for legacy naming.
using ScenarioCollection = scenario::ScenarioModel;

/// Options controlling instance id allocation.
struct InstanceGenerationOptions {
    common::InstanceId first_instance_id{1};
};

/// Utility that converts one scenario model into executable instances.
class ScenarioGenerator {
public:
    /**
     * @brief Builds one executable instance per task using a shared world definition.
     *
     * @param scenario_model Scenario model containing world and task set.
     * @param options Instance id allocation options.
     * @return Instance list preserving task order.
     */
    static std::vector<instance::InstanceModel> generate(
        const scenario::ScenarioModel& scenario_model,
        InstanceGenerationOptions options = {}
    );
};

}  // namespace skymapf::generator
