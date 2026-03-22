/**
 * @file obstacle_layout_strategy.cpp
 * @brief Implements built-in obstacle layout strategies.
 */
#include "skymapf/generator/strategy/obstacle_layout_strategy.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#include "skymapf/world/connectivity.hpp"

namespace skymapf::generator {

// Default strategy:
// 1) block all cells,
// 2) pick one random seed cell as walkable,
// 3) randomly carve frontier cells until target walkable count is reached.
bool ConnectedCarvingObstacleLayoutStrategy::apply(
    world::WorldModel& world_model,
    const ObstacleDensityConfig& config
) const {
    const auto total = world_model.cell_count();
    if (total == 0) {
        return false;
    }

    const auto density = std::clamp(config.obstacle_density, 0.0, 0.999);
    auto target_walkable = static_cast<std::size_t>(
        std::llround(static_cast<double>(total) * (1.0 - density))
    );
    target_walkable = std::max<std::size_t>(1, target_walkable);
    target_walkable = std::min<std::size_t>(target_walkable, static_cast<std::size_t>(total));

    // Start from a fully blocked map and carve one connected walkable region.
    for (common::CellIndex i = 0; i < total; ++i) {
        world_model.set_walkable(i, false);
    }

    std::mt19937_64 rng(config.random_seed);
    std::uniform_int_distribution<common::CellIndex> pick_cell(0, total - 1);
    const auto start = pick_cell(rng);
    world_model.set_walkable(start, true);

    std::vector<common::CellIndex> frontier;
    frontier.reserve(static_cast<std::size_t>(total));
    std::vector<std::uint8_t> in_frontier(static_cast<std::size_t>(total), 0u);

    const auto enqueue_blocked_neighbors = [&](common::CellIndex from) {
        for (const auto nb : world::geometric_neighbors(world_model.space_spec(), from)) {
            if (world_model.is_walkable(nb)) {
                continue;
            }
            auto& mark = in_frontier[static_cast<std::size_t>(nb)];
            if (mark != 0u) {
                continue;
            }
            mark = 1u;
            frontier.push_back(nb);
        }
    };

    enqueue_blocked_neighbors(start);
    std::size_t carved = 1;
    while (carved < target_walkable && !frontier.empty()) {
        std::uniform_int_distribution<std::size_t> pick_frontier(0, frontier.size() - 1);
        const auto idx = pick_frontier(rng);
        const auto cell = frontier[idx];
        frontier[idx] = frontier.back();
        frontier.pop_back();
        in_frontier[static_cast<std::size_t>(cell)] = 0u;

        if (world_model.is_walkable(cell)) {
            continue;
        }
        world_model.set_walkable(cell, true);
        ++carved;
        enqueue_blocked_neighbors(cell);
    }
    return carved == target_walkable;
}

}  // namespace skymapf::generator
