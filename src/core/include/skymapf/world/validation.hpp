#pragma once

#include <cstddef>
#include <vector>

#include "../common/ids.hpp"

namespace skymapf::world {

class WorldModel;

struct ConnectivityValidationOptions {
    // When true, an empty walkable subgraph is treated as connected.
    bool empty_is_connected = true;
    // Expected component count of walkable cells.
    std::size_t expected_component_count = 1;
};

struct ConnectivityValidationResult {
    bool is_fully_connected = true;
    bool passed = true;
    std::size_t component_count = 0;
    std::size_t walkable_cell_count = 0;
    std::size_t largest_component_size = 0;
};

std::size_t connected_component_count(const WorldModel& world);
bool is_fully_connected(const WorldModel& world);
bool has_single_connected_component(const WorldModel& world);
ConnectivityValidationResult validate_connectivity(
    const WorldModel& world,
    ConnectivityValidationOptions options
);
ConnectivityValidationResult validate_connectivity(const WorldModel& world);
std::vector<common::CellIndex> component_representatives(const WorldModel& world);

}  // namespace skymapf::world
