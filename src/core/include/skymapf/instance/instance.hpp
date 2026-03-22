/**
 * @file instance.hpp
 * @brief Defines executable instance model (world + single task).
 */
#pragma once

#include <string>
#include <utility>

#include "skymapf/common/ids.hpp"
#include "skymapf/task/task_model.hpp"
#include "skymapf/task/task_runtime.hpp"
#include "skymapf/world/world_model.hpp"

namespace skymapf::instance {

/**
 * @brief Represents one executable static instance.
 *
 * One instance contains one world and one task definition.
 */
class InstanceModel {
public:
    InstanceModel() = default;

    InstanceModel(
        common::InstanceId instance_id,
        world::WorldModel world,
        task::TaskModel task,
        std::string name = {}
    )
        : instance_id_(instance_id),
          world_(std::move(world)),
          task_(std::move(task)),
          name_(std::move(name)) {}

    static InstanceModel create(
        common::InstanceId instance_id,
        world::WorldModel world,
        task::TaskModel task,
        std::string name = {}
    ) {
        return InstanceModel(
            instance_id,
            std::move(world),
            std::move(task),
            std::move(name)
        );
    }

    common::InstanceId instance_id() const noexcept { return instance_id_; }
    void set_instance_id(common::InstanceId instance_id) noexcept { instance_id_ = instance_id; }

    const std::string& name() const noexcept { return name_; }
    void set_name(std::string name) { name_ = std::move(name); }

    const world::WorldModel& world() const noexcept { return world_; }
    world::WorldModel& world() noexcept { return world_; }
    void set_world(world::WorldModel world) { world_ = std::move(world); }

    const task::TaskModel& task() const noexcept { return task_; }
    task::TaskModel& task() noexcept { return task_; }
    void set_task(task::TaskModel task) { task_ = std::move(task); }

private:
    common::InstanceId instance_id_{0};
    world::WorldModel world_;
    task::TaskModel task_;
    std::string name_;
};

/// Runtime payload bound to one executable instance.
struct InstanceRuntimeState {
    common::InstanceId instance_id{0};
    common::TimeStep current_time{0};
    task::TaskRuntimeState task_runtime;
};

}  // namespace skymapf::instance
