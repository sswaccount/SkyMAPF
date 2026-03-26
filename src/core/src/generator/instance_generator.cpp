/**
 * @file instance_generator.cpp
 * @brief Implements one-step orchestration for instance generation.
 */
#include "skymapf/generator/instance_generator.hpp"

#include <string>
#include <utility>
#include <vector>

#include "skymapf/agent/model.hpp"
#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/random_tool.hpp"

namespace skymapf::generator {

instance::InstanceModel InstanceGenerator::generate(const InstanceGeneratorOptions& options) {
    auto world_model = WorldGenerator::generate(options.world_gen_options);
    auto task_model = TaskGenerator::generate(world_model, options.task_gen_options);

    std::vector<agent::AgentModel> agents;
    agents.reserve(task_model.agent_entries().size());
    for (const auto& entry : task_model.agent_entries()) {
        agents.emplace_back(entry.agent_id, utils::DefaultNaming::next_agent_name());
    }

    const auto instance_id = options.instance_id.value_or(
        utils::RandomTool::instance().next_id_value()
    );
    auto instance_name = options.instance_name.value_or(
        utils::DefaultNaming::next_instance_name()
    );

    return instance::InstanceModel::create(
        instance_id,
        std::move(world_model),
        std::move(agents),
        std::move(task_model),
        std::move(instance_name)
    );
}

}  // namespace skymapf::generator
