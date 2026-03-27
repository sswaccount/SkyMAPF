/**
 * @file analysis.cpp
 * @brief Implements world-level metrics for obstacle density and connectivity.
 */
#include "skymapf/world/analysis.hpp"

#include <cstddef>
#include <cstdint>
#include <queue>
#include <vector>

#include "skymapf/world/connectivity.hpp"
#include "skymapf/world/model.hpp"

namespace skymapf::world {

namespace {

std::vector<common::CellIndex> neighbors_by_mode(
    const WorldModel& world,
    common::CellIndex from,
    ComponentNeighborhoodMode mode
) {
    if (mode == ComponentNeighborhoodMode::ConnectivityPolicy) {
        return world.neighbors(from);
    }

    std::vector<common::CellIndex> result;
    for (const auto nb : geometric_neighbors(world.space_spec(), from)) {
        if (world.is_valid_index(nb) && world.is_walkable(nb)) {
            result.push_back(nb);
        }
    }
    return result;
}

std::size_t count_walkable_components(
    const WorldModel& world,
    ComponentNeighborhoodMode mode
) {
    const auto cell_count = world.cell_count();
    std::vector<std::uint8_t> visited(static_cast<std::size_t>(cell_count), 0u);
    std::queue<common::CellIndex> queue;

    std::size_t component_count = 0;
    for (common::CellIndex i = 0; i < cell_count; ++i) {
        if (!world.is_walkable(i)) {
            continue;
        }
        if (visited[static_cast<std::size_t>(i)] != 0u) {
            continue;
        }

        ++component_count;
        visited[static_cast<std::size_t>(i)] = 1u;
        queue.push(i);

        while (!queue.empty()) {
            const auto current = queue.front();
            queue.pop();
            for (const auto nb : neighbors_by_mode(world, current, mode)) {
                if (!world.is_valid_index(nb) || !world.is_walkable(nb)) {
                    continue;
                }
                auto& seen = visited[static_cast<std::size_t>(nb)];
                if (seen != 0u) {
                    continue;
                }
                seen = 1u;
                queue.push(nb);
            }
        }
    }

    return component_count;
}

}  // namespace

WorldAnalysisResult analyze_world(
    const WorldModel& world,
    ComponentNeighborhoodMode mode
) {
    WorldAnalysisResult result{};
    result.total_cells = world.cell_count();

    for (common::CellIndex i = 0; i < world.cell_count(); ++i) {
        if (world.is_walkable(i)) {
            ++result.walkable_cells;
        } else {
            ++result.obstacle_cells;
        }
    }

    if (result.total_cells != 0u) {
        result.obstacle_density =
            static_cast<double>(result.obstacle_cells) / static_cast<double>(result.total_cells);
    }
    result.walkable_component_count = count_walkable_components(world, mode);
    return result;
}

}  // namespace skymapf::world

