/**
 * @file scenario.hpp
 * @brief Defines static scenario model for dataset organization.
 */
#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../agent/model.hpp"
#include "../common/ids.hpp"
#include "../task/model.hpp"
#include "../utils/default_naming.hpp"
#include "../utils/random_tool.hpp"
#include "../world/model.hpp"

namespace skymapf::instance {
class InstanceModel;
}

namespace skymapf::scenario {

/**
 * @brief Optional scenario-level metadata for experiment/data management.
 */
struct ScenarioMetadata {
    std::string spec_version;
    std::string description;
};

/**
 * @brief Represents one static scenario data model.
 *
 * Scenario is an organization layer:
 * - one world
 * - one static agent catalog
 * - multiple task definitions
 * - optional metadata
 *
 * It is not required to be the direct solver input.
 */
class ScenarioModel {
public:
    ScenarioModel()
        : id_(utils::RandomTool::instance().next_id_value()),
          name_(utils::DefaultNaming::next_scenario_name()),
          world_(1, std::string{}, common::SpaceSpec::make_2d(1, 1)),
          agents_(),
          tasks_() {}

    /**
     * @brief Constructs a scenario model with explicit ownership.
     *
     * @param scenario_id Identifier of the scenario.
     * @param world World definition copied into the scenario.
     * @param tasks Task definitions copied into the scenario.
     * @param name Optional scenario display name.
     */
    ScenarioModel(
        common::ScenarioId id,
        std::string name,
        world::WorldModel world,
        std::vector<agent::AgentModel> agents,
        std::vector<task::TaskModel> tasks,
        ScenarioMetadata metadata = {}
    )
        : id_(id),
          name_(name.empty() ? utils::DefaultNaming::next_scenario_name() : std::move(name)),
          world_(std::move(world)),
          agents_(std::move(agents)),
          tasks_(std::move(tasks)),
          metadata_(std::move(metadata)) {}

    ScenarioModel(
        common::ScenarioId id,
        std::string name,
        world::WorldModel world,
        std::vector<task::TaskModel> tasks,
        ScenarioMetadata metadata = {}
    )
        : ScenarioModel(
            id,
            std::move(name),
            std::move(world),
            {},
            std::move(tasks),
            std::move(metadata)
        ) {}

    /// Creates a scenario model using value semantics.
    static ScenarioModel create(
        common::ScenarioId id,
        std::string name,
        world::WorldModel world,
        std::vector<agent::AgentModel> agents,
        std::vector<task::TaskModel> tasks,
        ScenarioMetadata metadata = {}
    ) {
        return ScenarioModel(
            id,
            std::move(name),
            std::move(world),
            std::move(agents),
            std::move(tasks),
            std::move(metadata)
        );
    }

    static ScenarioModel create(
        common::ScenarioId id,
        std::string name,
        world::WorldModel world,
        std::vector<task::TaskModel> tasks,
        ScenarioMetadata metadata = {}
    ) {
        return ScenarioModel(
            id,
            std::move(name),
            std::move(world),
            {},
            std::move(tasks),
            std::move(metadata)
        );
    }

    /// Returns the scenario identifier.
    const common::ScenarioId& id() const noexcept { return id_; }
    /// Sets the scenario identifier.
    void set_id(common::ScenarioId scenario_id) noexcept { id_ = scenario_id; }

    /// Returns the scenario name.
    const std::string& name() const noexcept { return name_; }
    /// Returns whether scenario display name is non-empty.
    bool has_name() const noexcept { return !name_.empty(); }
    /// Sets the scenario name.
    void set_name(std::string name) { name_ = std::move(name); }

    /// Returns the owned world (const view).
    const world::WorldModel& world() const noexcept { return world_; }
    /// Returns the owned world (mutable view).
    world::WorldModel& world() noexcept { return world_; }
    /// Replaces the owned world by value.
    void set_world(world::WorldModel world) { world_ = std::move(world); }

    /// Returns the owned static agent catalog (const view).
    const std::vector<agent::AgentModel>& agents() const noexcept { return agents_; }
    /// Returns the owned static agent catalog (mutable view).
    std::vector<agent::AgentModel>& agents() noexcept { return agents_; }
    /// Replaces the owned agent catalog by value.
    void set_agents(std::vector<agent::AgentModel> agents) { agents_ = std::move(agents); }

    /// Returns the owned tasks (const view).
    const std::vector<task::TaskModel>& tasks() const noexcept { return tasks_; }
    /// Returns the owned tasks (mutable view).
    std::vector<task::TaskModel>& tasks() noexcept { return tasks_; }
    /// Replaces the owned tasks by value.
    void set_tasks(std::vector<task::TaskModel> tasks) { tasks_ = std::move(tasks); }
    /// Clears all tasks.
    void clear_tasks() { tasks_.clear(); }
    /// Appends one task.
    void add_task(task::TaskModel task) { tasks_.push_back(std::move(task)); }

    /// Returns scenario metadata.
    const ScenarioMetadata& metadata() const noexcept { return metadata_; }
    /// Sets scenario metadata.
    void set_metadata(ScenarioMetadata metadata) { metadata_ = std::move(metadata); }

    /**
     * @brief Exports one executable instance per task.
     *
     * @param first_instance_id First instance id in generated output.
     * @return Generated executable instances preserving task order.
     */
    std::vector<instance::InstanceModel> export_instances(
        common::InstanceId first_instance_id = 1
    ) const;

    /**
     * @brief Exports one executable instance selected by task id.
     *
     * @param task_id Task id to select.
     * @param instance_id Instance id assigned to output instance.
     * @param instance_name Optional output instance name.
     * @return Exported instance when task exists; std::nullopt otherwise.
     */
    std::optional<instance::InstanceModel> export_instance_by_task_id(
        common::TaskId task_id,
        common::InstanceId instance_id = 1,
        std::string instance_name = {}
    ) const;

private:
    common::ScenarioId id_;
    std::string name_;
    world::WorldModel world_;
    std::vector<agent::AgentModel> agents_;
    std::vector<task::TaskModel> tasks_;
    ScenarioMetadata metadata_;
};

}  // namespace skymapf::scenario
