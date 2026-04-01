/**
 * @file checksum.cpp
 * @brief Implements lightweight checksum helpers.
 */
#include "skymapf/utils/checksum.hpp"

#include <cstdint>
#include <iomanip>
#include <sstream>

namespace skymapf::utils {

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

std::string build_scenario_meta_checksum_payload(
    common::ScenarioId scenario_id,
    const std::string& name,
    const std::string& info,
    std::uint64_t random_seed,
    common::WorldId world_id,
    const std::vector<common::TaskId>& task_ids
) {
    std::ostringstream oss;
    oss << scenario_id << '|'
        << name << '|'
        << info << '|'
        << random_seed << '|'
        << world_id << '|';
    for (const auto task_id : task_ids) {
        oss << task_id << ',';
    }
    return oss.str();
}

}  // namespace skymapf::utils
