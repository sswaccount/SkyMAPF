/**
 * @file analysis.hpp
 * @brief Provides world-level metrics such as obstacle density and component count.
 */
#pragma once

#include <cstddef>
#include <cstdint>

namespace skymapf::world {

class WorldModel;

/// Selects how neighbor relations are interpreted for connectivity analysis.
enum class ComponentNeighborhoodMode {
    /// Uses geometric neighborhood (4-neighbor in 2D, 6-neighbor in 3D).
    Geometric,
    /// Uses the active world connectivity policy via WorldModel::neighbors.
    ConnectivityPolicy,
};

/// Aggregated world metrics used for testing and diagnostics.
struct WorldAnalysisResult {
    std::uint64_t total_cells{0};
    std::uint64_t obstacle_cells{0};
    std::uint64_t walkable_cells{0};
    double obstacle_density{0.0};
    std::size_t walkable_component_count{0};
};

/**
 * @brief Computes obstacle density and walkable connected-component count.
 *
 * Obstacle density is computed as:
 *   obstacle_cells / total_cells
 *
 * Connected components are counted only among walkable cells. For most map
 * quality checks, prefer ComponentNeighborhoodMode::Geometric.
 */
WorldAnalysisResult analyze_world(
    const WorldModel& world,
    ComponentNeighborhoodMode mode = ComponentNeighborhoodMode::Geometric
);

}  // namespace skymapf::world

