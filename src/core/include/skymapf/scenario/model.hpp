/**
 * @file scenario.hpp
 * @brief Defines static scenario model for dataset organization.
 */
#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <cstdint>

#include <nlohmann/json.hpp>

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
 * @brief Scenario-level metadata summary used for indexing and validation.
 */
struct ScenarioMeta {
    common::ScenarioId scenario_id{0};          ///< Scenario identifier.
    std::string name;                           ///< Scenario display name.
    std::string info;                           ///< Optional scenario description text.
    common::WorldId world_id{0};                ///< Referenced world identifier.
    std::vector<common::TaskId> task_ids;       ///< Task identifiers in scenario order.
    std::string checksum;                       ///< Integrity checksum of meta payload.

    /**
     * @brief Serializes scenario meta to ordered JSON.
     *
     * @param j Output JSON object.
     * @param meta Scenario metadata to serialize.
     */
    friend void to_json(nlohmann::ordered_json& j, const ScenarioMeta& meta);

    /**
     * @brief Deserializes scenario meta from JSON.
     *
     * @param j Input JSON object.
     * @param meta Output scenario metadata.
     */
    friend void from_json(const nlohmann::json& j, ScenarioMeta& meta);
};

/**
 * @brief Represents one static scenario data model.
 *
 * Scenario is an organization layer:
 * - one world
 * - multiple task definitions
 * - optional metadata
 *
 * It is not required to be the direct solver input.
 */
class ScenarioModel {
public:
    /**
     * @brief Constructs a scenario with auto-generated id and name.
     *
     * @param world World definition copied into the scenario.
     * @param tasks Task definitions copied into the scenario.
     */
    ScenarioModel(
        world::WorldModel world,
        std::vector<task::TaskModel> tasks = {}
    )
        : ScenarioModel(
            utils::RandomTool::instance().next_id_value(),
            utils::DefaultNaming::next_scenario_name(),
            std::move(world),
            std::move(tasks)
        ) {}

    /**
     * @brief Constructs a scenario model with explicit ownership.
     *
     * @param id Identifier of the scenario.
     * @param name Optional scenario display name.
     * @param world World definition copied into the scenario.
     * @param tasks Task definitions copied into the scenario.
     */
    ScenarioModel(
        common::ScenarioId id,
        std::string name,
        world::WorldModel world,
        std::vector<task::TaskModel> tasks
    )
        : id_(id),
          name_(name.empty() ? utils::DefaultNaming::next_scenario_name() : std::move(name)),
          random_seed_(utils::RandomTool::instance().root_seed()),
          world_(std::move(world)),
          tasks_(std::move(tasks)) {}

    /**
     * @brief Factory helper creating a scenario model by value.
     */
    static ScenarioModel create(
        common::ScenarioId id,
        std::string name,
        world::WorldModel world,
        std::vector<task::TaskModel> tasks
    ) {
        return ScenarioModel(
            id,
            std::move(name),
            std::move(world),
            std::move(tasks)
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
    /// Returns additional scenario description text.
    const std::string& info() const noexcept { return info_; }
    /// Returns whether additional scenario description text is non-empty.
    bool has_info() const noexcept { return !info_.empty(); }
    /// Sets additional scenario description text.
    void set_info(std::string info) { info_ = std::move(info); }
    /// Returns scenario-level random seed used for generation metadata.
    std::uint64_t random_seed() const noexcept { return random_seed_; }
    /// Sets scenario-level random seed used for generation metadata.
    void set_random_seed(std::uint64_t random_seed) noexcept { random_seed_ = random_seed; }

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
     * @brief Builds a metadata summary view for this scenario.
     *
     * The returned payload includes:
     * - scenario id, name, info
     * - world id
     * - ordered task id list
     * - checksum over the above fields
     */
    ScenarioMeta meta() const;

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
    std::string info_;
    std::uint64_t random_seed_{1};
    world::WorldModel world_;
    std::vector<task::TaskModel> tasks_;
};

}  // namespace skymapf::scenario
