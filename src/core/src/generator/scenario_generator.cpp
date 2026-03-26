/**
 * @file scenario_generator.cpp
 * @brief Implements orchestration from generation options to scenario model.
 */
#include "skymapf/generator/scenario_generator.hpp"

#include <unordered_set>
#include <vector>

#include "skymapf/agent/model.hpp"
#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/random_tool.hpp"

namespace skymapf::generator {

scenario::ScenarioModel ScenarioGenerator::generate(
    const ScenarioGeneratorOptions& options
) {
    auto world_model = WorldGenerator::generate(options.world_gen_options);
    auto task_models = TaskGenerator::generate(world_model, options.tasks_gen_options);

    std::unordered_set<common::AgentId> seen_ids;
    std::vector<agent::AgentModel> agents;
    for (const auto& task_model : task_models) {
        for (const auto& entry : task_model.agent_entries()) {
            if (!seen_ids.insert(entry.agent_id).second) {
                continue;
            }
            agents.emplace_back(entry.agent_id, utils::DefaultNaming::next_agent_name());
        }
    }

    const auto scenario_id = options.scenario_id.value_or(
        utils::RandomTool::instance().next_id_value()
    );
    auto scenario_name = options.scenario_name.value_or(
        utils::DefaultNaming::next_scenario_name()
    );

    return scenario::ScenarioModel::create(
        scenario_id,
        std::move(scenario_name),
        std::move(world_model),
        std::move(agents),
        std::move(task_models)
    );
}

}  // namespace skymapf::generator
