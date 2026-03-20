/**
 * @file scenario_generator.cpp
 * @brief Implements conversion from world/task collections to scenarios.
 */
#include "skymapf/generator/scenario_generator.hpp"

namespace skymapf::generator {

std::vector<scenario::Scenario> ScenarioGenerator::build_scenarios(
    const ScenarioCollection& collection,
    ScenarioBuildOptions options
) {
    std::vector<scenario::Scenario> scenarios;
    scenarios.reserve(collection.tasks().size());

    auto scenario_id = options.first_scenario_id;
    for (const auto& task_data : collection.tasks()) {
        common::TimeStep start_time = 0;
        if (options.use_agent_start_time_as_start_time) {
            const auto earliest = task_data.earliest_start_time();
            if (earliest.has_value()) {
                start_time = *earliest;
            }
        }
        scenarios.push_back(scenario::Scenario::create(
            scenario_id++,
            collection.world(),
            task_data,
            start_time,
            task_data.name()
        ));
    }
    return scenarios;
}

}  // namespace skymapf::generator
