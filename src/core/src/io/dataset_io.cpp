#include "skymapf/io/dataset_io.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>

#include <nlohmann/json.hpp>

#include "skymapf/common/space.hpp"

namespace skymapf::io {

namespace {

using json = nlohmann::json;

constexpr const char* kMetaFile = "meta.json";
constexpr const char* kWorldFile = "world.json";
constexpr const char* kTasksFile = "tasks.json";

std::string fnv1a64_hex(std::string_view text) {
    std::uint64_t hash = 14695981039346656037ull;
    for (const auto ch : text) {
        hash ^= static_cast<std::uint8_t>(ch);
        hash *= 1099511628211ull;
    }
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(16) << hash;
    return oss.str();
}

std::string status_to_string(task::TaskStatus status) {
    switch (status) {
    case task::TaskStatus::Pending:
        return "pending";
    case task::TaskStatus::Active:
        return "active";
    case task::TaskStatus::Completed:
        return "completed";
    case task::TaskStatus::Failed:
        return "failed";
    default:
        return "pending";
    }
}

task::TaskStatus task_status_from_string(const std::string& text) {
    if (text == "active") {
        return task::TaskStatus::Active;
    }
    if (text == "completed") {
        return task::TaskStatus::Completed;
    }
    if (text == "failed") {
        return task::TaskStatus::Failed;
    }
    return task::TaskStatus::Pending;
}

json world_to_json(const world::WorldModel& world_model) {
    json j;
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

    world::WorldModel world_model(spec, false);
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

json task_to_json(const task::Task& task_data) {
    json j;
    j["task_id"] = task_data.task_id();
    j["name"] = task_data.name();
    j["status"] = status_to_string(task_data.status());
    j["timing"] = {
        {"release_time", task_data.timing().release_time},
        {"deadline", task_data.timing().deadline.has_value()
                         ? json(*task_data.timing().deadline)
                         : json(nullptr)},
    };

    json sequences = json::array();
    for (const auto& seq : task_data.agent_sequences()) {
        sequences.push_back({
            {"agent_id", seq.agent_id},
            {"checkpoints", seq.visit_sequence.checkpoints},
        });
    }
    j["agent_sequences"] = std::move(sequences);
    return j;
}

std::optional<task::Task> task_from_json(const json& j, std::string* error_message) {
    if (!j.contains("task_id")) {
        if (error_message) {
            *error_message = "task entry missing 'task_id'.";
        }
        return std::nullopt;
    }
    task::TaskTiming timing;
    if (j.contains("timing")) {
        const auto& t = j.at("timing");
        timing.release_time = t.value("release_time", 0u);
        if (t.contains("deadline") && !t.at("deadline").is_null()) {
            timing.deadline = t.at("deadline").get<common::TimeStep>();
        }
    }

    task::Task task_data = task::Task::create(
        j.at("task_id").get<common::TaskId>(),
        timing,
        j.value("name", std::string{})
    );
    task_data.set_status(task_status_from_string(j.value("status", std::string{"pending"})));

    if (!j.contains("agent_sequences") || !j.at("agent_sequences").is_array()) {
        if (error_message) {
            *error_message = "task entry missing 'agent_sequences'.";
        }
        return std::nullopt;
    }
    for (const auto& seq : j.at("agent_sequences")) {
        const auto agent_id = seq.at("agent_id").get<common::AgentId>();
        const auto checkpoints = seq.at("checkpoints").get<std::vector<common::CellIndex>>();
        task::VisitSequence visit_sequence;
        visit_sequence.checkpoints = checkpoints;
        task_data.upsert_agent_sequence(agent_id, std::move(visit_sequence));
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

    std::vector<task::Task> tasks;
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

std::vector<scenario::Scenario> DatasetIO::make_scenarios(
    const DatasetData& data,
    common::ScenarioId first_scenario_id
) {
    std::vector<scenario::Scenario> scenarios;
    scenarios.reserve(data.tasks.size());
    common::ScenarioId scenario_id = first_scenario_id;
    for (const auto& task_data : data.tasks) {
        common::TimeStep time = 0;
        if (task_data.timing().release_time != 0) {
            time = task_data.timing().release_time;
        }
        scenarios.push_back(scenario::Scenario::create(
            scenario_id++,
            data.world,
            task_data,
            time,
            task_data.name()
        ));
    }
    return scenarios;
}

std::optional<std::vector<scenario::Scenario>> DatasetIO::read_dataset_as_scenarios(
    const std::filesystem::path& directory,
    common::ScenarioId first_scenario_id,
    std::string* error_message
) {
    auto dataset = read_dataset(directory, error_message);
    if (!dataset.has_value()) {
        return std::nullopt;
    }
    return make_scenarios(dataset->data, first_scenario_id);
}

}  // namespace skymapf::io
