/**
 * @file validation.hpp
 * @brief Declares connectivity validation helpers for world models.
 */
#pragma once

#include <cstddef>
#include <vector>

#include "../common/ids.hpp"

namespace skymapf::world {

class WorldModel;

/// Options that control world connectivity validation behavior.
struct ConnectivityValidationOptions {
    // When true, an empty walkable subgraph is treated as connected.
    bool empty_is_connected = true;
    // Expected component count of walkable cells.
    std::size_t expected_component_count = 1;
};

/// Aggregated statistics produced by connectivity validation.
struct ConnectivityValidationResult {
    bool is_fully_connected = true;
    bool passed = true;
    std::size_t component_count = 0;
    std::size_t walkable_cell_count = 0;
    std::size_t largest_component_size = 0;
};

/// Returns the number of connected components among walkable cells.
std::size_t connected_component_count(const WorldModel& world);
/// Returns true when walkable cells satisfy the configured connectivity condition.
bool is_fully_connected(const WorldModel& world);
/// Returns true when exactly one walkable connected component exists.
bool has_single_connected_component(const WorldModel& world);
/**
 * @brief Validates connectivity and returns detailed metrics.
 *
 * @param world World to analyze.
 * @param options Connectivity acceptance criteria.
 * @return Validation statistics and pass/fail status.
 */
ConnectivityValidationResult validate_connectivity(
    const WorldModel& world,
    ConnectivityValidationOptions options
);
/// Validates connectivity using default options.
ConnectivityValidationResult validate_connectivity(const WorldModel& world);
/// Returns one representative index per connected walkable component.
std::vector<common::CellIndex> component_representatives(const WorldModel& world);

}  // namespace skymapf::world
