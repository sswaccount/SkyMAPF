/**
 * @file instance.hpp
 * @brief Defines executable instance model (world + agents + single task).
 */
#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include <utility>

#include "skymapf/agent/model.hpp"
#include "skymapf/common/ids.hpp"
#include "skymapf/task/model.hpp"
#include "skymapf/task/runtime.hpp"
#include "skymapf/utils/default_naming.hpp"
#include "skymapf/utils/random_tool.hpp"
#include "skymapf/world/model.hpp"

namespace skymapf::scenario {
class ScenarioModel;
}

namespace skymapf::instance {

/**
 * @brief Represents one executable static instance.
 *
 * One instance binds:
 * - static world
 * - static agent identities
 * - one task demand aggregate
 *
 * It also maintains resolved agent-task binding metadata so future
 * solver/runtime modules can consume assignment relations directly.
 */
class InstanceModel {
public:
    struct ResolvedAgentTask {
        common::AgentId agent_id{0};
        std::size_t agent_index{0};
        std::size_t task_entry_index{0};
    };

    InstanceModel()
        : id_(utils::RandomTool::instance().next_id_value()),
          name_(utils::DefaultNaming::next_instance_name()),
          world_(1, std::string{}, common::SpaceSpec::make_2d(1, 1)),
          task_() {
        rebuild_resolved_bindings();
    }

    InstanceModel(
        common::InstanceId instance_id,
        world::WorldModel world,
        std::vector<agent::AgentModel> agents,
        task::TaskModel task,
        std::string name = {}
    )
        : id_(instance_id),
          name_(name.empty() ? utils::DefaultNaming::next_instance_name() : std::move(name)),
          world_(std::move(world)),
          agents_(std::move(agents)),
          task_(std::move(task)) {
        rebuild_resolved_bindings();
    }

    InstanceModel(
        common::InstanceId instance_id,
        world::WorldModel world,
        task::TaskModel task,
        std::string name = {}
    )
        : InstanceModel(
            instance_id,
            std::move(world),
            derive_agents_from_task(task),
            std::move(task),
            std::move(name)
        ) {}

    static InstanceModel create(
        common::InstanceId instance_id,
        world::WorldModel world,
        std::vector<agent::AgentModel> agents,
        task::TaskModel task,
        std::string name = {}
    ) {
        return InstanceModel(
            instance_id,
            std::move(world),
            std::move(agents),
            std::move(task),
            std::move(name)
        );
    }

    static InstanceModel create(
        common::InstanceId instance_id,
        world::WorldModel world,
        task::TaskModel task,
        std::string name = {}
    ) {
        return InstanceModel(
            instance_id,
            std::move(world),
            derive_agents_from_task(task),
            std::move(task),
            std::move(name)
        );
    }

    const common::InstanceId& id() const noexcept { return id_; }
    void set_id(common::InstanceId instance_id) noexcept { id_ = instance_id; }

    const std::string& name() const noexcept { return name_; }
    bool has_name() const noexcept { return !name_.empty(); }
    void set_name(std::string name) { name_ = std::move(name); }

    const world::WorldModel& world() const noexcept { return world_; }
    world::WorldModel& world() noexcept { return world_; }
    void set_world(world::WorldModel world) { world_ = std::move(world); }

    const std::vector<agent::AgentModel>& agents() const noexcept { return agents_; }
    std::vector<agent::AgentModel>& agents() noexcept { return agents_; }
    void set_agents(std::vector<agent::AgentModel> agents) {
        agents_ = std::move(agents);
        rebuild_resolved_bindings();
    }

    const task::TaskModel& task() const noexcept { return task_; }
    task::TaskModel& task() noexcept { return task_; }
    void set_task(task::TaskModel task) {
        task_ = std::move(task);
        rebuild_resolved_bindings();
    }

    const agent::AgentModel* find_agent(common::AgentId agent_id) const noexcept;
    const task::AgentTaskEntry* find_agent_task(common::AgentId agent_id) const noexcept;
    const ResolvedAgentTask* find_assignment(common::AgentId agent_id) const noexcept;
    const std::vector<ResolvedAgentTask>& assignments() const noexcept { return resolved_bindings_; }
    bool has_consistent_assignments() const noexcept;

    /// Rebuilds resolved binding metadata after external mutable edits.
    void rebuild_resolved_bindings();

    /**
     * @brief Converts this instance into a single-task scenario.
     *
     * The resulting scenario keeps the same world and stores exactly one task.
     *
     * @param scenario_id Output scenario id.
     * @param scenario_name Optional output scenario name.
     * @return Generated single-task scenario model.
     */
    scenario::ScenarioModel to_scenario(
        common::ScenarioId scenario_id = 1,
        std::string scenario_name = {}
    ) const;

private:
    static std::vector<agent::AgentModel> derive_agents_from_task(const task::TaskModel& task) {
        std::vector<agent::AgentModel> derived_agents;
        derived_agents.reserve(task.agent_entries().size());
        for (const auto& entry : task.agent_entries()) {
            derived_agents.emplace_back(entry.agent_id, utils::DefaultNaming::next_agent_name());
        }
        return derived_agents;
    }

    common::InstanceId id_;
    std::string name_;
    world::WorldModel world_;
    std::vector<agent::AgentModel> agents_;
    task::TaskModel task_;
    std::vector<ResolvedAgentTask> resolved_bindings_;
};

/// Runtime payload bound to one executable instance.
struct InstanceRuntimeState {
    common::InstanceId instance_id{0};
    common::TimeStep current_time{0};
    task::TaskRuntimeState task_runtime;
};

}  // namespace skymapf::instance
