/**
 * @file scenario.hpp
 * @brief Defines the scenario object that binds world, task, and time.
 */
#pragma once

#include <string>
#include <utility>

#include "../common/ids.hpp"
#include "../task/task.hpp"
#include "../world/world_model.hpp"

namespace skymapf::scenario {

/**
 * @brief Represents one executable scenario instance.
 *
 * A scenario owns one world and one task together with the current
 * simulation time used by runtime and solver pipelines.
 */
class Scenario {
public:
    Scenario() = default;

    /**
     * @brief Constructs a scenario object with explicit ownership.
     *
     * @param scenario_id Identifier of the scenario instance.
     * @param world World definition copied into the scenario.
     * @param task Task definition copied into the scenario.
     * @param current_time Initial simulation time.
     * @param name Optional scenario display name.
     */
    Scenario(
        common::ScenarioId scenario_id,
        world::WorldModel world,
        task::Task task,
        common::TimeStep current_time = 0,
        std::string name = {}
    )
        : scenario_id_(scenario_id),
          world_(std::move(world)),
          task_(std::move(task)),
          current_time_(current_time),
          name_(std::move(name)) {}

    /// Creates a scenario using value semantics.
    static Scenario create(
        common::ScenarioId scenario_id,
        world::WorldModel world,
        task::Task task,
        common::TimeStep current_time = 0,
        std::string name = {}
    ) {
        return Scenario(
            scenario_id,
            std::move(world),
            std::move(task),
            current_time,
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

    /// Returns the owned task (const view).
    const task::Task& task() const noexcept { return task_; }
    /// Returns the owned task (mutable view).
    task::Task& task() noexcept { return task_; }
    /// Replaces the owned task by value.
    void set_task(task::Task task) { task_ = std::move(task); }

    /// Returns current scenario time.
    common::TimeStep current_time() const noexcept { return current_time_; }
    /// Sets current scenario time.
    void set_current_time(common::TimeStep time_step) noexcept { current_time_ = time_step; }
    /// Advances scenario time by @p delta ticks.
    void advance_time(common::TimeStep delta = 1) noexcept { current_time_ += delta; }

    /// Returns whether the task is available at current scenario time.
    bool is_task_available() const noexcept {
        return task_.is_available(current_time_);
    }

private:
    common::ScenarioId scenario_id_{0};
    world::WorldModel world_;
    task::Task task_;
    common::TimeStep current_time_{0};
    std::string name_;
};

}  // namespace skymapf::scenario
