#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "../scenario/scenario.hpp"
#include "../task/task.hpp"
#include "../world/world_model.hpp"

namespace skymapf::io {

struct DatasetMeta {
    std::string schema_version{"skymapf.data.v1"};
    std::string dataset_name;
};

struct DatasetData {
    DatasetMeta meta;
    world::WorldModel world;
    std::vector<task::Task> tasks;
};

struct DatasetManifest {
    std::string checksum;
    std::uint64_t task_count{0};
};

struct DatasetReadStatus {
    bool checksum_valid{true};
    std::string stored_checksum;
    std::string recomputed_checksum;
};

struct DatasetReadResult {
    DatasetData data;
    DatasetManifest manifest;
    DatasetReadStatus status;
};

class DatasetIO {
public:
    // Writes dataset into directory:
    //   <dir>/meta.json
    //   <dir>/world.json
    //   <dir>/tasks.json
    static bool write_dataset(
        const DatasetData& data,
        const std::filesystem::path& directory,
        DatasetManifest* written_manifest = nullptr,
        std::string* error_message = nullptr
    );

    static std::optional<DatasetReadResult> read_dataset(
        const std::filesystem::path& directory,
        std::string* error_message = nullptr
    );

    // Creates one scenario per task; each scenario reuses logically same world data.
    // scenario.current_time defaults to 0, but if task.release_time != 0 it uses release_time.
    static std::vector<scenario::Scenario> make_scenarios(
        const DatasetData& data,
        common::ScenarioId first_scenario_id = 1
    );

    static std::optional<std::vector<scenario::Scenario>> read_dataset_as_scenarios(
        const std::filesystem::path& directory,
        common::ScenarioId first_scenario_id = 1,
        std::string* error_message = nullptr
    );
};

}  // namespace skymapf::io
