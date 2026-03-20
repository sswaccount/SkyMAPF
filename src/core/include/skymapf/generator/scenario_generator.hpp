#pragma once

#include <string>
#include <utility>
#include <vector>

#include "../common/ids.hpp"
#include "../scenario/scenario.hpp"
#include "../task/task.hpp"
#include "../world/world_model.hpp"

namespace skymapf::generator {

// Decoupled storage that binds one world with multiple tasks.
class ScenarioCollection {
public:
    ScenarioCollection() = default;
    explicit ScenarioCollection(world::WorldModel world, std::string name = {})
        : world_(std::move(world)), name_(std::move(name)) {}

    const world::WorldModel& world() const noexcept { return world_; }
    world::WorldModel& world() noexcept { return world_; }
    void set_world(world::WorldModel world) { world_ = std::move(world); }

    const std::vector<task::Task>& tasks() const noexcept { return tasks_; }
    std::vector<task::Task>& tasks() noexcept { return tasks_; }
    void clear_tasks() { tasks_.clear(); }
    void add_task(task::Task task) { tasks_.push_back(std::move(task)); }

    const std::string& name() const noexcept { return name_; }
    void set_name(std::string name) { name_ = std::move(name); }

private:
    world::WorldModel world_;
    std::vector<task::Task> tasks_;
    std::string name_;
};

struct ScenarioBuildOptions {
    common::ScenarioId first_scenario_id{1};
    bool use_task_release_time_as_start_time{true};
};

class ScenarioGenerator {
public:
    static std::vector<scenario::Scenario> build_scenarios(
        const ScenarioCollection& collection,
        ScenarioBuildOptions options = {}
    );
};

}  // namespace skymapf::generator
