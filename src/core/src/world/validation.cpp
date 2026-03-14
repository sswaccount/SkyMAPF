/**
 * @file validation.cpp
 * @brief Implements world connectivity validation.
 */
#include "skymapf/world/validation.hpp"

#include <cstdint>
#include <queue>
#include <vector>
#include "skymapf/world/model.hpp"

namespace skymapf::world {

namespace {

std::size_t connected_component_count(const WorldModel& world) {
    const auto cell_count = world.cell_count();
    std::vector<std::uint8_t> visited(static_cast<std::size_t>(cell_count), 0u);
    std::queue<common::CellIndex> queue;
    std::size_t component_count = 0;

    for (common::CellIndex index = 0; index < cell_count; ++index) {
        if (!world.is_walkable(index)) {
            continue;
        }
        if (visited[static_cast<std::size_t>(index)] != 0u) {
            continue;
        }

        ++component_count;

        visited[static_cast<std::size_t>(index)] = 1u;
        queue.push(index);

        while (!queue.empty()) {
            const auto current = queue.front();
            queue.pop();

            for (const auto neighbor : world.neighbors(current)) {
                if (!world.is_valid_index(neighbor) || !world.is_walkable(neighbor)) {
                    continue;
                }
                auto& seen = visited[static_cast<std::size_t>(neighbor)];
                if (seen != 0u) {
                    continue;
                }
                seen = 1u;
                queue.push(neighbor);
            }
        }
    }

    return component_count;
}

}  // namespace

ConnectivityValidationStatus ConnectivityValidator::validate(const WorldModel& world) {
    return connected_component_count(world) == 1
        ? ConnectivityValidationStatus::Success
        : ConnectivityValidationStatus::Failed;
}

}  // namespace skymapf::world
