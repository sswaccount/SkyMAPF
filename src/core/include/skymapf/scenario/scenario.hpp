#pragma once

#include <string>
#include <utility>

#include "../common/ids.hpp"
#include "../task/task.hpp"
#include "../world/world_model.hpp"

namespace skymapf::scenario {

class Scenario {
public:
    Scenario() = default;

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

    common::ScenarioId scenario_id() const noexcept { return scenario_id_; }
    void set_scenario_id(common::ScenarioId scenario_id) noexcept { scenario_id_ = scenario_id; }

    const std::string& name() const noexcept { return name_; }
    void set_name(std::string name) { name_ = std::move(name); }

    const world::WorldModel& world() const noexcept { return world_; }
    world::WorldModel& world() noexcept { return world_; }
    void set_world(world::WorldModel world) { world_ = std::move(world); }

    const task::Task& task() const noexcept { return task_; }
    task::Task& task() noexcept { return task_; }
    void set_task(task::Task task) { task_ = std::move(task); }

    common::TimeStep current_time() const noexcept { return current_time_; }
    void set_current_time(common::TimeStep time_step) noexcept { current_time_ = time_step; }
    void advance_time(common::TimeStep delta = 1) noexcept { current_time_ += delta; }

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
