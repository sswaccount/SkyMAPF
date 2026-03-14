/**
 * @file scenario_io.hpp
 * @brief Declares scenario-level filesystem read/write helpers.
 */
#pragma once

#include <filesystem>
#include <string>

#include "skymapf/scenario/model.hpp"

namespace skymapf::io {

/**
 * @brief Filesystem-backed serializer/deserializer for ScenarioModel.
 */
class ScenarioIO {
public:
    /**
     * @brief Constructs one IO helper bound to a root directory.
     *
     * Each scenario is stored under one subdirectory of @p root_directory.
     *
     * @param root_directory Base directory for scenario folders.
     */
    explicit ScenarioIO(std::filesystem::path root_directory);

    /**
     * @brief Returns root directory used by this IO helper.
     */
    const std::filesystem::path& root_directory() const noexcept { return root_directory_; }

    /**
     * @brief Writes one scenario into `<root>/<scenario_name>/`.
     *
     * Output files:
     * - `meta.json`
     * - `<world_name>.json`
     * - `<task_name>.json` for each task
     *
     * @param scenario Scenario source object.
     * @return Created scenario directory path.
     * @throws std::runtime_error On IO or serialization failure.
     */
    std::filesystem::path write(const scenario::ScenarioModel& scenario) const;

    /**
     * @brief Reads one scenario by folder name under root directory.
     *
     * The loader parses world/task files referenced by `meta.json` and validates
     * id/name/info/world_id/task_ids/checksum consistency.
     *
     * @param scenario_name Folder name under root directory.
     * @return Parsed scenario model.
     * @throws std::runtime_error On read/parse/validation failure.
     */
    scenario::ScenarioModel read(const std::string& scenario_name) const;

private:
    std::filesystem::path root_directory_;
};

}  // namespace skymapf::io
