/**
 * @file scenario_io.cpp
 * @brief Implements scenario-level filesystem read/write helpers.
 */
#include "skymapf/io/scenario_io.hpp"

#include <fstream>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <unordered_set>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "skymapf/io/json_format.hpp"
#include "skymapf/utils/checksum.hpp"
#include "skymapf/utils/default_naming.hpp"

namespace skymapf::io {

namespace {

using json = nlohmann::ordered_json;

struct ScenarioDiskMeta {
    std::string schema_version{"skymapf.scenario.v1"};
    std::uint64_t random_seed{0};
    scenario::ScenarioMeta scenario_meta;
    std::string world_file;
    std::vector<std::string> task_files;
};

void to_json(json& j, const ScenarioDiskMeta& meta) {
    j = json{
        {"schema_version", meta.schema_version},
        {"random_seed", meta.random_seed},
        {"scenario_meta", meta.scenario_meta},
        {"world_file", meta.world_file},
        {"task_files", meta.task_files}
    };
}

void from_json(const nlohmann::json& j, ScenarioDiskMeta& meta) {
    if (!j.contains("scenario_meta")) {
        throw std::runtime_error("meta.json missing 'scenario_meta'.");
    }
    if (!j.contains("random_seed")) {
        throw std::runtime_error("meta.json missing 'random_seed'.");
    }
    if (!j.contains("world_file")) {
        throw std::runtime_error("meta.json missing 'world_file'.");
    }
    if (!j.contains("task_files")) {
        throw std::runtime_error("meta.json missing 'task_files'.");
    }
    meta.schema_version = j.value("schema_version", std::string{"skymapf.scenario.v1"});
    meta.random_seed = j.at("random_seed").get<std::uint64_t>();
    meta.scenario_meta = j.at("scenario_meta").get<scenario::ScenarioMeta>();
    meta.world_file = j.at("world_file").get<std::string>();
    meta.task_files = j.at("task_files").get<std::vector<std::string>>();
}

std::string sanitize_filename(std::string text) {
    for (auto& ch : text) {
        const bool invalid = (ch == '<' || ch == '>' || ch == ':' || ch == '"' ||
            ch == '/' || ch == '\\' || ch == '|' || ch == '?' || ch == '*');
        if (invalid || static_cast<unsigned char>(ch) < 0x20u) {
            ch = '_';
        }
    }
    while (!text.empty() && (text.back() == ' ' || text.back() == '.')) {
        text.pop_back();
    }
    if (text.empty()) {
        return "unnamed";
    }
    return text;
}

std::string choose_scenario_folder_name(const scenario::ScenarioModel& scenario) {
    if (scenario.has_name()) {
        return sanitize_filename(scenario.name());
    }
    return sanitize_filename(utils::DefaultNaming::next_scenario_name());
}

std::string choose_world_file_name(const world::WorldModel& world) {
    if (world.has_name()) {
        return sanitize_filename(world.name()) + ".json";
    }
    return "world_" + std::to_string(world.id()) + ".json";
}

std::vector<std::string> choose_task_file_names(const std::vector<task::TaskModel>& tasks) {
    std::vector<std::string> files;
    files.reserve(tasks.size());
    std::unordered_set<std::string> used;
    for (std::size_t i = 0; i < tasks.size(); ++i) {
        const auto& task = tasks[i];
        std::string base = task.has_name()
            ? sanitize_filename(task.name())
            : ("task_" + std::to_string(task.id()));
        if (base.empty()) {
            base = "task";
        }
        std::string name = base + ".json";
        if (used.find(name) != used.end()) {
            name = base + "_" + std::to_string(task.id()) + ".json";
        }
        if (used.find(name) != used.end()) {
            name = base + "_" + std::to_string(i) + ".json";
        }
        used.insert(name);
        files.push_back(std::move(name));
    }
    return files;
}

void write_json_file(const std::filesystem::path& path, const json& value) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("Cannot open file for writing: " + path.string());
    }
    out << dump_json_inline_arrays(value, 2);
}

nlohmann::json read_json_file(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open file for reading: " + path.string());
    }
    nlohmann::json value;
    try {
        in >> value;
    } catch (const std::exception& ex) {
        throw std::runtime_error("Invalid JSON in " + path.string() + ": " + ex.what());
    }
    return value;
}

void validate_meta(
    const ScenarioDiskMeta& disk_meta,
    const scenario::ScenarioModel& scenario_model
) {
    const auto recomputed = scenario_model.meta();
    if (disk_meta.scenario_meta.scenario_id != recomputed.scenario_id) {
        throw std::runtime_error("Scenario meta validation failed: scenario_id mismatch.");
    }
    if (disk_meta.scenario_meta.name != recomputed.name) {
        throw std::runtime_error("Scenario meta validation failed: name mismatch.");
    }
    if (disk_meta.scenario_meta.info != recomputed.info) {
        throw std::runtime_error("Scenario meta validation failed: info mismatch.");
    }
    if (disk_meta.random_seed != scenario_model.random_seed()) {
        throw std::runtime_error("Scenario meta validation failed: random_seed mismatch.");
    }
    if (disk_meta.scenario_meta.world_id != recomputed.world_id) {
        throw std::runtime_error("Scenario meta validation failed: world_id mismatch.");
    }
    if (disk_meta.scenario_meta.task_ids != recomputed.task_ids) {
        throw std::runtime_error("Scenario meta validation failed: task_ids mismatch.");
    }
    if (disk_meta.scenario_meta.checksum != recomputed.checksum) {
        throw std::runtime_error("Scenario meta validation failed: checksum mismatch.");
    }
}

}  // namespace

ScenarioIO::ScenarioIO(std::filesystem::path root_directory)
    : root_directory_(std::move(root_directory)) {}

std::filesystem::path ScenarioIO::write(const scenario::ScenarioModel& scenario) const {
    std::error_code ec;
    std::filesystem::create_directories(root_directory_, ec);
    if (ec) {
        throw std::runtime_error("Failed to create root directory: " + root_directory_.string());
    }

    const auto folder_name = choose_scenario_folder_name(scenario);
    const auto scenario_dir = root_directory_ / folder_name;
    std::filesystem::create_directories(scenario_dir, ec);
    if (ec) {
        throw std::runtime_error("Failed to create scenario directory: " + scenario_dir.string());
    }

    const auto world_file = choose_world_file_name(scenario.world());
    const auto task_files = choose_task_file_names(scenario.tasks());

    ScenarioDiskMeta disk_meta;
    disk_meta.random_seed = scenario.random_seed();
    disk_meta.scenario_meta = scenario.meta();
    disk_meta.world_file = world_file;
    disk_meta.task_files = task_files;

    write_json_file(scenario_dir / "meta.json", json(disk_meta));
    write_json_file(scenario_dir / world_file, json(scenario.world()));
    for (std::size_t i = 0; i < scenario.tasks().size(); ++i) {
        write_json_file(scenario_dir / task_files[i], json(scenario.tasks()[i]));
    }

    return scenario_dir;
}

scenario::ScenarioModel ScenarioIO::read(const std::string& scenario_name) const {
    const auto scenario_dir = root_directory_ / scenario_name;
    if (!std::filesystem::exists(scenario_dir)) {
        throw std::runtime_error("Scenario directory not found: " + scenario_dir.string());
    }

    const auto meta_json = read_json_file(scenario_dir / "meta.json");
    const auto disk_meta = meta_json.get<ScenarioDiskMeta>();

    const auto world_json = read_json_file(scenario_dir / disk_meta.world_file);
    auto world_model = world::WorldModel(
        common::SpaceSpec::make_2d(1, 1),
        world::WorldModel::OccupancyFactory{},
        world::WorldModel::ConnectivityFactory{}
    );
    from_json(world_json, world_model);

    std::vector<task::TaskModel> tasks;
    tasks.reserve(disk_meta.task_files.size());
    for (const auto& file_name : disk_meta.task_files) {
        const auto task_json = read_json_file(scenario_dir / file_name);
        tasks.push_back(task_json.get<task::TaskModel>());
    }

    auto scenario_model = scenario::ScenarioModel::create(
        disk_meta.scenario_meta.scenario_id,
        disk_meta.scenario_meta.name,
        std::move(world_model),
        std::move(tasks)
    );
    scenario_model.set_info(disk_meta.scenario_meta.info);
    scenario_model.set_random_seed(disk_meta.random_seed);
    validate_meta(disk_meta, scenario_model);
    return scenario_model;
}

}  // namespace skymapf::io
