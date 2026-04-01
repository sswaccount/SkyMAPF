/**
 * @file checksum.hpp
 * @brief Defines lightweight checksum helpers used by metadata serialization.
 */
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "skymapf/common/ids.hpp"

namespace skymapf::utils {

/**
 * @brief Computes FNV-1a 64-bit checksum and returns lower-case hex text.
 *
 * @param text Input payload to hash.
 * @return 16-character lower-case hex checksum.
 */
std::string fnv1a64_hex(std::string_view text);

/**
 * @brief Builds canonical scenario-meta checksum payload text.
 *
 * The output is intended to be passed to fnv1a64_hex.
 *
 * @param scenario_id Scenario identifier.
 * @param name Scenario name.
 * @param info Scenario info text.
 * @param world_id Referenced world identifier.
 * @param task_ids Ordered task id list.
 * @return Canonical plain-text payload.
 */
std::string build_scenario_meta_checksum_payload(
    common::ScenarioId scenario_id,
    const std::string& name,
    const std::string& info,
    std::uint64_t random_seed,
    common::WorldId world_id,
    const std::vector<common::TaskId>& task_ids
);

}  // namespace skymapf::utils
