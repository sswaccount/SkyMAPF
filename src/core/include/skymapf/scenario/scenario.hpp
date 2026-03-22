/**
 * @file scenario.hpp
 * @brief Defines static scenario model that binds world and multiple tasks.
 */
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "../common/ids.hpp"
#include "../task/task_model.hpp"
#include "../world/world_model.hpp"

namespace skymapf::instance {
class InstanceModel;
}

namespace skymapf::scenario {

/**
 * @brief Represents one static scenario data model.
 *
 * A scenario contains one world and multiple task definitions.
 */
class ScenarioModel {
public:
    ScenarioModel() = default;

    /**
     * @brief Constructs a scenario model with explicit ownership.
     *
     * @param scenario_id Identifier of the scenario.
     * @param world World definition copied into the scenario.
     * @param tasks Task definitions copied into the scenario.
     * @param name Optional scenario display name.
     */
    ScenarioModel(
        common::ScenarioId scenario_id,
        world::WorldModel world,
        std::vector<task::TaskModel> tasks,
        std::string name = {}
    )
        : scenario_id_(scenario_id),
          world_(std::move(world)),
          tasks_(std::move(tasks)),
          name_(std::move(name)) {}

    /// Creates a scenario model using value semantics.
    static ScenarioModel create(
        common::ScenarioId scenario_id,
        world::WorldModel world,
        std::vector<task::TaskModel> tasks,
        std::string name = {}
    ) {
        return ScenarioModel(
            scenario_id,
            std::move(world),
            std::move(tasks),
            std::move(name)
        );
    }

    /// Returns the scenario identifier.
    common::ScenarioId scenario_id() const noexcept { return scenario_id_; }
    /// Sets the scenario identifier.
    void set_scenario_id(common::ScenarioId scenario_id) noexcept { scenario_id_ = scenario_id; }

    /// Returns the scenario name.
    const std::string& name() const noexcept { return name_; }
    /// Sets the scenario name.
    void set_name(std::string name) { name_ = std::move(name); }

    /// Returns the owned world (const view).
    const world::WorldModel& world() const noexcept { return world_; }
    /// Returns the owned world (mutable view).
    world::WorldModel& world() noexcept { return world_; }
    /// Replaces the owned world by value.
    void set_world(world::WorldModel world) { world_ = std::move(world); }

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

    /**
     * @brief Exports one executable instance per task.
     *
     * @param first_instance_id First instance id in generated output.
     * @return Generated executable instances preserving task order.
     */
    std::vector<instance::InstanceModel> export_instances(
        common::InstanceId first_instance_id = 1
    ) const;

private:
    common::ScenarioId scenario_id_{0};
    world::WorldModel world_;
    std::vector<task::TaskModel> tasks_;
    std::string name_;
};

}  // namespace skymapf::scenario
