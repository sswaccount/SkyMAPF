/**
 * @file scenario_generator.hpp
 * @brief Defines collection and conversion helpers from world/tasks to scenarios.
 */
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "../common/ids.hpp"
#include "../scenario/scenario.hpp"
#include "../task/task.hpp"
#include "../world/world_model.hpp"

namespace skymapf::generator {

/**
 * @brief Decoupled storage that binds one world with multiple tasks.
 */
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

/// Options controlling scenario id allocation and initial time semantics.
struct ScenarioBuildOptions {
    common::ScenarioId first_scenario_id{1};
    bool use_agent_start_time_as_start_time{true};
};

/// Utility that converts world/task collections into scenario instances.
class ScenarioGenerator {
public:
    /**
     * @brief Builds one scenario per task using a shared world definition.
     *
     * @param collection World/task collection.
     * @param options Scenario id and initial-time settings.
     * @return Scenario list preserving task order.
     */
    static std::vector<scenario::Scenario> build_scenarios(
        const ScenarioCollection& collection,
        ScenarioBuildOptions options = {}
    );
};

}  // namespace skymapf::generator
