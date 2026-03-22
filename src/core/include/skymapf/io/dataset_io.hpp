/**
 * @file dataset_io.hpp
 * @brief Declares dataset-level JSON read/write APIs for world and task data.
 */
#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "../instance/model.hpp"
#include "../scenario/model.hpp"
#include "../task/model.hpp"
#include "../world/model.hpp"

namespace skymapf::io {

/// Human-readable metadata stored with a dataset.
struct DatasetMeta {
    std::string schema_version{"skymapf.data.v1"};
    std::string dataset_name;
};

/// Payload data serialized in dataset files.
struct DatasetData {
    DatasetMeta meta;
    world::WorldModel world;
    std::vector<task::TaskModel> tasks;
};

/// Manifest values derived from serialized payload.
struct DatasetManifest {
    std::string checksum;
    std::uint64_t task_count{0};
};

/// Runtime status produced while reading and validating dataset files.
struct DatasetReadStatus {
    bool checksum_valid{true};
    std::string stored_checksum;
    std::string recomputed_checksum;
};

/// Full result returned by dataset read operations.
struct DatasetReadResult {
    DatasetData data;
    DatasetManifest manifest;
    DatasetReadStatus status;
};

/**
 * @brief Filesystem-backed dataset serializer/deserializer.
 */
class DatasetIO {
public:
    // Writes dataset into directory:
    //   <dir>/meta.json
    //   <dir>/world.json
    //   <dir>/tasks.json
    /**
     * @brief Writes a dataset into a directory of JSON files.
     *
     * @param data Data payload to serialize.
     * @param directory Output directory path.
     * @param written_manifest Optional output manifest.
     * @param error_message Optional error output on failure.
     * @return True on success; false otherwise.
     */
    static bool write_dataset(
        const DatasetData& data,
        const std::filesystem::path& directory,
        DatasetManifest* written_manifest = nullptr,
        std::string* error_message = nullptr
    );

    /**
     * @brief Reads a dataset directory and validates checksum metadata.
     *
     * @param directory Input directory path.
     * @param error_message Optional error output on failure.
     * @return Parsed dataset and read status when successful.
     */
    static std::optional<DatasetReadResult> read_dataset(
        const std::filesystem::path& directory,
        std::string* error_message = nullptr
    );

    /**
     * @brief Builds one scenario model from one dataset payload.
     *
     * @param data Dataset payload.
     * @param scenario_id Scenario identifier.
     * @param name Optional scenario name.
     */
    static scenario::ScenarioModel make_scenario_model(
        const DatasetData& data,
        common::ScenarioId scenario_id = 1,
        std::string name = {}
    );

    /**
     * @brief Builds one executable instance per task from dataset payload.
     */
    static std::vector<instance::InstanceModel> make_instances(
        const DatasetData& data,
        common::InstanceId first_instance_id = 1
    );

    /**
     * @brief Reads a dataset and directly converts it into executable instances.
     */
    static std::optional<std::vector<instance::InstanceModel>> read_dataset_as_instances(
        const std::filesystem::path& directory,
        common::InstanceId first_instance_id = 1,
        std::string* error_message = nullptr
    );
};

}  // namespace skymapf::io
