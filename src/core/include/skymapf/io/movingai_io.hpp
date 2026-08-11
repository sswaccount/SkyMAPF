/**
 * @file movingai_io.hpp
 * @brief Declares compatibility import for MovingAI MAPF benchmarks.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "skymapf/common/ids.hpp"
#include "skymapf/instance/model.hpp"
#include "skymapf/world/model.hpp"

namespace skymapf::io {

/// One row from a MovingAI `.scen` file.
struct MovingAIScenarioEntry {
    std::uint64_t bucket{0};
    std::string map_name;
    std::int32_t map_width{0};
    std::int32_t map_height{0};
    std::int32_t start_x{0};
    std::int32_t start_y{0};
    std::int32_t goal_x{0};
    std::int32_t goal_y{0};
    double reference_distance{0.0};
};

/// Stable identities assigned while converting external benchmark files.
struct MovingAIImportOptions {
    common::WorldId world_id{1};
    common::TaskId first_task_id{1};
    common::InstanceId first_instance_id{1};
};

/**
 * @brief Imports MovingAI `.map` and `.scen` files into SkyMAPF models.
 *
 * MovingAI map occupancy is preserved and converted to a four-neighbor graph,
 * matching the `movingai_standard_v1` protocol semantics. Scenario prefix-k
 * conversion uses rows `[0, k)` as agents with stable ids `[1, k]`.
 */
class MovingAIIO {
public:
    static std::optional<world::WorldModel> read_map(
        const std::filesystem::path& map_path,
        common::WorldId world_id = 1,
        std::string* error_message = nullptr
    );

    static std::optional<std::vector<MovingAIScenarioEntry>> read_scenario(
        const std::filesystem::path& scenario_path,
        std::string* error_message = nullptr
    );

    static std::optional<instance::InstanceModel> make_prefix_instance(
        const world::WorldModel& world,
        const std::vector<MovingAIScenarioEntry>& entries,
        std::size_t agent_count,
        common::TaskId task_id,
        common::InstanceId instance_id,
        std::string instance_name = {},
        std::string* error_message = nullptr
    );

    static std::optional<std::vector<instance::InstanceModel>> read_prefix_instances(
        const std::filesystem::path& map_path,
        const std::filesystem::path& scenario_path,
        const std::vector<std::size_t>& agent_counts,
        const MovingAIImportOptions& options = {},
        std::string* error_message = nullptr
    );
};

}  // namespace skymapf::io
