#include "skymapf/world/validation.hpp"

#include <algorithm>
#include <cstdint>
#include <queue>
#include <vector>

#include "skymapf/world/world_model.hpp"

namespace skymapf::world {

namespace {

struct ConnectivityAnalysis {
    ConnectivityValidationResult result;
    std::vector<common::CellIndex> representatives;
};

ConnectivityAnalysis analyze_connectivity(
    const WorldModel& world,
    bool collect_representatives
) {
    ConnectivityAnalysis analysis;
    const auto cell_count = world.cell_count();
    std::vector<std::uint8_t> visited(static_cast<std::size_t>(cell_count), 0u);
    std::queue<common::CellIndex> queue;

    for (common::CellIndex index = 0; index < cell_count; ++index) {
        if (world.is_walkable(index)) {
            ++analysis.result.walkable_cell_count;
        }
    }

    for (common::CellIndex index = 0; index < cell_count; ++index) {
        if (!world.is_walkable(index)) {
            continue;
        }
        if (visited[static_cast<std::size_t>(index)] != 0u) {
            continue;
        }

        ++analysis.result.component_count;
        if (collect_representatives) {
            analysis.representatives.push_back(index);
        }

        visited[static_cast<std::size_t>(index)] = 1u;
        queue.push(index);
        std::size_t component_size = 0;

        while (!queue.empty()) {
            const auto current = queue.front();
            queue.pop();
            ++component_size;

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

        analysis.result.largest_component_size =
            std::max(analysis.result.largest_component_size, component_size);
    }

    analysis.result.is_fully_connected =
        analysis.result.walkable_cell_count == 0 || analysis.result.component_count == 1;

    return analysis;
}

}  // namespace

std::size_t connected_component_count(const WorldModel& world) {
    return validate_connectivity(world).component_count;
}

bool is_fully_connected(const WorldModel& world) {
    return validate_connectivity(world).is_fully_connected;
}

bool has_single_connected_component(const WorldModel& world) {
    return validate_connectivity(world).component_count == 1;
}

ConnectivityValidationResult validate_connectivity(
    const WorldModel& world,
    ConnectivityValidationOptions options
) {
    auto result = analyze_connectivity(world, false).result;
    if (result.walkable_cell_count == 0 && options.empty_is_connected) {
        result.is_fully_connected = true;
    } else {
        result.is_fully_connected = (result.component_count == options.expected_component_count);
    }
    result.passed = result.is_fully_connected;
    return result;
}

ConnectivityValidationResult validate_connectivity(const WorldModel& world) {
    return validate_connectivity(world, ConnectivityValidationOptions{});
}

std::vector<common::CellIndex> component_representatives(const WorldModel& world) {
    return analyze_connectivity(world, true).representatives;
}

}  // namespace skymapf::world
