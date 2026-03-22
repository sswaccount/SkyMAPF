/**
 * @file instance_generator.cpp
 * @brief Implements one-step orchestration for instance generation.
 */
#include "skymapf/generator/instance_generator.hpp"

#include <string>
#include <utility>

#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/id_generator.hpp"

namespace skymapf::generator {

instance::InstanceModel InstanceGenerator::generate(const InstanceGeneratorOptions& options) {
    const auto randomization = options.randomization.value_or(
        options.task_gen_options.randomization.value_or(
            options.task_gen_options.random_seed == 0
                ? options.world_gen_options.randomization.value_or(
                      options.world_gen_options.obstacle_config.random_seed == 0
                          ? common::RandomizationContext::make_default()
                          : common::RandomizationContext::make_deterministic(
                                options.world_gen_options.obstacle_config.random_seed
                            )
                  )
                : common::RandomizationContext::make_deterministic(options.task_gen_options.random_seed)
        )
    );

    auto world_options = options.world_gen_options;
    if (!world_options.randomization.has_value()) {
        world_options.randomization = randomization;
    }
    auto task_options = options.task_gen_options;
    if (!task_options.randomization.has_value()) {
        if (task_options.random_seed != 0) {
            task_options.randomization = common::RandomizationContext::make_deterministic(
                task_options.random_seed
            );
        } else {
            task_options.randomization = randomization;
        }
    }

    auto world_model = WorldGenerator::generate(world_options);
    auto task_model = TaskGenerator::generate(world_model, task_options);

    const auto instance_id = options.instance_id.value_or(
        utils::IdGenerator::deterministic_instance_id(
            randomization,
            static_cast<std::uint64_t>(task_model.id()) ^ static_cast<std::uint64_t>(world_model.id())
        )
    );
    auto instance_name = options.instance_name.value_or(
        utils::DefaultNaming::instance_name(randomization, instance_id)
    );

    return instance::InstanceModel::create(
        instance_id,
        std::move(world_model),
        std::move(task_model),
        std::move(instance_name)
    );
}

}  // namespace skymapf::generator
