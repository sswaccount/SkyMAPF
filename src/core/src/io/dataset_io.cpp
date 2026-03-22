/**
 * @file dataset_io.cpp
 * @brief Implements filesystem-backed JSON dataset serialization.
 */
#include "skymapf/io/dataset_io.hpp"

#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>

#include <nlohmann/json.hpp>

#include "skymapf/common/space.hpp"
#include "skymapf/world/occupancy.hpp"

namespace skymapf::io {

namespace {

using json = nlohmann::json;

constexpr const char* kMetaFile = "meta.json";
constexpr const char* kWorldFile = "world.json";
constexpr const char* kTasksFile = "tasks.json";

std::string fnv1a64_hex(std::string_view text) {
    // A lightweight non-cryptographic checksum is sufficient for corruption detection.
    std::uint64_t hash = 14695981039346656037ull;
    for (const auto ch : text) {
        hash ^= static_cast<std::uint8_t>(ch);
        hash *= 1099511628211ull;
    }
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(16) << hash;
    return oss.str();
}

std::string goal_behavior_to_string(task::GoalArrivalBehavior behavior) {
    switch (behavior) {
    case task::GoalArrivalBehavior::DisappearAtGoal:
        return "disappear_at_goal";
    case task::GoalArrivalBehavior::StayAtGoal:
        return "stay_at_goal";
    default:
        return "stay_at_goal";
    }
}

task::GoalArrivalBehavior goal_behavior_from_string(const std::string& text) {
    if (text == "disappear_at_goal") {
        return task::GoalArrivalBehavior::DisappearAtGoal;
    }
    return task::GoalArrivalBehavior::StayAtGoal;
}

json world_to_json(const world::WorldModel& world_model) {
    json j;
    j["world_id"] = world_model.id();
    j["name"] = world_model.name();
    const auto& spec = world_model.space_spec();
    const auto kind = spec.is_3d() ? "3d" : "2d";
    j["space"] = {
        {"kind", kind},
        {"cols", spec.cols()},
        {"rows", spec.rows()},
        {"layers", spec.layers()},
    };
    json walkable = json::array();
    walkable.get_ref<json::array_t&>().reserve(static_cast<std::size_t>(world_model.cell_count()));
    for (common::CellIndex idx = 0; idx < world_model.cell_count(); ++idx) {
        if (world_model.is_walkable(idx)) {
            walkable.push_back(idx);
        }
    }
    j["walkable_indices"] = std::move(walkable);
    j["connectivity"] = {{"type", "grid_adjacency"}};
    return j;
}

std::optional<world::WorldModel> world_from_json(const json& j, std::string* error_message) {
    if (!j.contains("space")) {
        if (error_message) {
            *error_message = "world.json missing 'space'.";
        }
        return std::nullopt;
    }
    const auto& s = j.at("space");
    const auto kind = s.value("kind", "2d");
    const auto cols = s.value("cols", 0);
    const auto rows = s.value("rows", 0);
    const auto layers = s.value("layers", 1);

    common::SpaceSpec spec;
    if (kind == "3d") {
        spec = common::SpaceSpec::make_3d(cols, rows, layers);
    } else {
        spec = common::SpaceSpec::make_2d(cols, rows);
    }
    if (!spec.is_valid()) {
        if (error_message) {
            *error_message = "world.json has invalid space shape.";
        }
        return std::nullopt;
    }

    const auto parsed_world_id = j.value("world_id", static_cast<common::WorldId>(0));
    const auto parsed_world_name = j.value("name", std::string{});
    world::WorldModel world_model(
        spec,
        [](common::CellIndex cell_count) {
            auto occupancy = std::make_shared<world::DefaultOccupancyStore>(cell_count);
            for (common::CellIndex index = 0; index < occupancy->cell_count(); ++index) {
                occupancy->set_walkable(index, false);
            }
            return occupancy;
        },
        {},
        parsed_world_id == 0 ? std::nullopt : std::optional<common::WorldId>{parsed_world_id},
        parsed_world_name.empty() ? std::nullopt : std::optional<std::string>{parsed_world_name}
    );
    if (!j.contains("walkable_indices") || !j.at("walkable_indices").is_array()) {
        if (error_message) {
            *error_message = "world.json missing array 'walkable_indices'.";
        }
        return std::nullopt;
    }
    for (const auto& item : j.at("walkable_indices")) {
        const auto idx = item.get<common::CellIndex>();
        if (world_model.is_valid_index(idx)) {
            world_model.set_walkable(idx, true);
        }
    }
    return world_model;
}

json task_to_json(const task::TaskModel& task_data) {
    json j;
    j["task_id"] = task_data.task_id();
    j["name"] = task_data.name();

    json sequences = json::array();
    for (const auto& seq : task_data.agent_sequences()) {
        sequences.push_back({
            {"agent_id", seq.agent_id},
            {"start_time", seq.start_time},
            {"goal_behavior", goal_behavior_to_string(seq.goal_behavior)},
            {"checkpoints", seq.visit_sequence.checkpoints},
        });
    }
    j["agent_sequences"] = std::move(sequences);
    return j;
}

std::optional<task::TaskModel> task_from_json(const json& j, std::string* error_message) {
    if (!j.contains("task_id")) {
        if (error_message) {
            *error_message = "task entry missing 'task_id'.";
        }
        return std::nullopt;
    }
    task::TaskModel task_data = task::TaskModel::create(
        j.at("task_id").get<common::TaskId>(),
        j.value("name", std::string{})
    );

    if (!j.contains("agent_sequences") || !j.at("agent_sequences").is_array()) {
        if (error_message) {
            *error_message = "task entry missing 'agent_sequences'.";
        }
        return std::nullopt;
    }
    for (const auto& seq : j.at("agent_sequences")) {
        const auto agent_id = seq.at("agent_id").get<common::AgentId>();
        const auto start_time = seq.value("start_time", static_cast<common::TimeStep>(0));
        task::GoalArrivalBehavior goal_behavior = task::GoalArrivalBehavior::StayAtGoal;
        if (seq.contains("goal_behavior")) {
            goal_behavior = goal_behavior_from_string(seq.value("goal_behavior", std::string{"stay_at_goal"}));
        }
        const auto checkpoints = seq.at("checkpoints").get<std::vector<common::CellIndex>>();
        task::SequenceModel visit_sequence;
        visit_sequence.checkpoints = checkpoints;
        task_data.upsert_agent_sequence(agent_id, std::move(visit_sequence), start_time, goal_behavior);
    }
    return task_data;
}

bool write_json_file(const std::filesystem::path& path, const json& j, std::string* error_message) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        if (error_message) {
            *error_message = "Cannot open file for writing: " + path.string();
        }
        return false;
    }
    out << j.dump(2);
    return true;
}

std::optional<json> read_json_file(const std::filesystem::path& path, std::string* error_message) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        if (error_message) {
            *error_message = "Cannot open file for reading: " + path.string();
        }
        return std::nullopt;
    }
    json j;
    try {
        in >> j;
    } catch (const std::exception& ex) {
        if (error_message) {
            *error_message = "Invalid JSON in " + path.string() + ": " + ex.what();
        }
        return std::nullopt;
    }
    return j;
}

}  // namespace

bool DatasetIO::write_dataset(
    const DatasetData& data,
    const std::filesystem::path& directory,
    DatasetManifest* written_manifest,
    std::string* error_message
) {
    std::error_code ec;
    std::filesystem::create_directories(directory, ec);
    if (ec) {
        if (error_message) {
            *error_message = "Failed to create directory: " + directory.string();
        }
        return false;
    }

    const auto world_json = world_to_json(data.world);
    json tasks_json = json::array();
    tasks_json.get_ref<json::array_t&>().reserve(data.tasks.size());
    for (const auto& task_data : data.tasks) {
        tasks_json.push_back(task_to_json(task_data));
    }
    const auto checksum = fnv1a64_hex(world_json.dump() + tasks_json.dump());

    DatasetManifest manifest;
    manifest.task_count = static_cast<std::uint64_t>(data.tasks.size());
    manifest.checksum = checksum;
    if (written_manifest) {
        *written_manifest = manifest;
    }
    json meta_json = {
        {"schema_version", data.meta.schema_version},
        {"dataset_name", data.meta.dataset_name},
        {"checksum", manifest.checksum},
        {"task_count", manifest.task_count},
    };

    if (!write_json_file(directory / kMetaFile, meta_json, error_message)) {
        return false;
    }
    if (!write_json_file(directory / kWorldFile, world_json, error_message)) {
        return false;
    }
    if (!write_json_file(directory / kTasksFile, tasks_json, error_message)) {
        return false;
    }
    return true;
}

std::optional<DatasetReadResult> DatasetIO::read_dataset(
    const std::filesystem::path& directory,
    std::string* error_message
) {
    const auto meta_json = read_json_file(directory / kMetaFile, error_message);
    if (!meta_json.has_value()) {
        return std::nullopt;
    }
    const auto world_json = read_json_file(directory / kWorldFile, error_message);
    if (!world_json.has_value()) {
        return std::nullopt;
    }
    const auto tasks_json = read_json_file(directory / kTasksFile, error_message);
    if (!tasks_json.has_value()) {
        return std::nullopt;
    }
    if (!tasks_json->is_array()) {
        if (error_message) {
            *error_message = "tasks.json root must be an array.";
        }
        return std::nullopt;
    }

    auto world_model = world_from_json(*world_json, error_message);
    if (!world_model.has_value()) {
        return std::nullopt;
    }

    std::vector<task::TaskModel> tasks;
    tasks.reserve(tasks_json->size());
    for (const auto& task_item : *tasks_json) {
        auto parsed_task = task_from_json(task_item, error_message);
        if (!parsed_task.has_value()) {
            return std::nullopt;
        }
        tasks.push_back(std::move(*parsed_task));
    }

    DatasetReadResult result;
    result.data.world = std::move(*world_model);
    result.data.tasks = std::move(tasks);
    result.data.meta.schema_version = meta_json->value("schema_version", std::string{"skymapf.data.v1"});
    result.data.meta.dataset_name = meta_json->value("dataset_name", std::string{});
    result.manifest.checksum = meta_json->value("checksum", std::string{});
    result.manifest.task_count =
        meta_json->value("task_count", static_cast<std::uint64_t>(result.data.tasks.size()));

    result.status.stored_checksum = result.manifest.checksum;
    result.status.recomputed_checksum = fnv1a64_hex(world_json->dump() + tasks_json->dump());
    result.status.checksum_valid = result.status.stored_checksum.empty()
        ? false
        : (result.status.recomputed_checksum == result.status.stored_checksum);
    return result;
}

scenario::ScenarioModel DatasetIO::make_scenario_model(
    const DatasetData& data,
    common::ScenarioId scenario_id,
    std::string name
) {
    if (name.empty()) {
        name = data.meta.dataset_name;
    }
    return scenario::ScenarioModel::create(
        scenario_id,
        std::move(name),
        data.world,
        data.tasks
    );
}

std::vector<instance::InstanceModel> DatasetIO::make_instances(
    const DatasetData& data,
    common::InstanceId first_instance_id
) {
    const auto scenario_model = make_scenario_model(data);
    return scenario_model.export_instances(first_instance_id);
}

std::optional<std::vector<instance::InstanceModel>> DatasetIO::read_dataset_as_instances(
    const std::filesystem::path& directory,
    common::InstanceId first_instance_id,
    std::string* error_message
) {
    auto dataset = read_dataset(directory, error_message);
    if (!dataset.has_value()) {
        return std::nullopt;
    }
    return make_instances(dataset->data, first_instance_id);
}

}  // namespace skymapf::io
