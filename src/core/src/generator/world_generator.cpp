#include "skymapf/generator/world_generator.hpp"

#include <algorithm>
#include <cmath>
#include <queue>
#include <random>
#include <unordered_set>
#include <vector>

#include "skymapf/common/index.hpp"
#include "skymapf/common/move.hpp"
#include "skymapf/world/validation.hpp"

namespace skymapf::generator {

namespace {

std::vector<common::CellIndex> geometric_neighbors(
    const common::SpaceSpec& spec,
    common::CellIndex index
) {
    std::vector<common::CellIndex> out;
    if (spec.is_2d()) {
        const auto shape = spec.shape_2d();
        const auto c = common::to_coord_2d(index, shape);
        out.reserve(common::kMoveDelta4.size());
        for (const auto& d : common::kMoveDelta4) {
            const common::CellCoord2D n{c.x + d.x, c.y + d.y};
            if (!common::is_in_bounds(n, shape)) {
                continue;
            }
            out.push_back(common::to_index(n, shape));
        }
        return out;
    }

    const auto shape = spec.shape_3d();
    const auto c = common::to_coord_3d(index, shape);
    out.reserve(common::kMoveDelta6.size());
    for (const auto& d : common::kMoveDelta6) {
        const common::CellCoord3D n{c.x + d.x, c.y + d.y, c.z + d.z};
        if (!common::is_in_bounds(n, shape)) {
            continue;
        }
        out.push_back(common::to_index(n, shape));
    }
    return out;
}

std::vector<std::vector<common::CellIndex>> compute_components(const world::WorldModel& world_model) {
    const auto total = world_model.cell_count();
    std::vector<std::uint8_t> visited(static_cast<std::size_t>(total), 0u);
    std::vector<std::vector<common::CellIndex>> components;
    std::queue<common::CellIndex> q;

    for (common::CellIndex i = 0; i < total; ++i) {
        if (!world_model.is_walkable(i) || visited[static_cast<std::size_t>(i)] != 0u) {
            continue;
        }
        components.emplace_back();
        auto& comp = components.back();

        visited[static_cast<std::size_t>(i)] = 1u;
        q.push(i);
        while (!q.empty()) {
            const auto cur = q.front();
            q.pop();
            comp.push_back(cur);
            for (const auto nb : world_model.neighbors(cur)) {
                if (visited[static_cast<std::size_t>(nb)] != 0u || !world_model.is_walkable(nb)) {
                    continue;
                }
                visited[static_cast<std::size_t>(nb)] = 1u;
                q.push(nb);
            }
        }
    }
    return components;
}

}  // namespace

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
        for (const auto nb : geometric_neighbors(world_model.space_spec(), from)) {
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
    return carved >= 1;
}

void KeepLargestComponentRepairStrategy::repair(world::WorldModel& world_model) const {
    auto components = compute_components(world_model);
    if (components.size() <= 1) {
        return;
    }
    const auto it = std::max_element(
        components.begin(),
        components.end(),
        [](const auto& lhs, const auto& rhs) { return lhs.size() < rhs.size(); }
    );
    std::unordered_set<common::CellIndex> keep(it->begin(), it->end());

    for (common::CellIndex i = 0; i < world_model.cell_count(); ++i) {
        if (!world_model.is_walkable(i)) {
            continue;
        }
        if (keep.find(i) == keep.end()) {
            world_model.set_walkable(i, false);
        }
    }
}

WorldGenerationResult WorldGenerator::generate(const WorldGenerationRequest& request) {
    WorldGenerationResult result;
    result.world = world::WorldModel(request.space_spec, true);

    auto obstacle_strategy = request.obstacle_strategy;
    if (!obstacle_strategy) {
        obstacle_strategy = std::make_shared<ConnectedCarvingObstacleLayoutStrategy>();
    }
    auto repair_strategy = request.repair_strategy;
    if (!repair_strategy) {
        repair_strategy = std::make_shared<KeepLargestComponentRepairStrategy>();
    }

    const auto attempts = std::max<std::size_t>(1, request.max_attempts);
    for (std::size_t attempt = 0; attempt < attempts; ++attempt) {
        result.attempts_used = attempt + 1;
        auto world_model = world::WorldModel(request.space_spec, true);

        ObstacleDensityConfig config = request.obstacle_config;
        config.random_seed += static_cast<std::uint64_t>(attempt * 104729);
        if (!obstacle_strategy->apply(world_model, config)) {
            continue;
        }

        if (request.require_single_component && !world::is_fully_connected(world_model)) {
            repair_strategy->repair(world_model);
        }

        const auto connected_now = world::is_fully_connected(world_model);
        if (!request.require_single_component || connected_now) {
            result.world = std::move(world_model);
            result.connected = connected_now;
            return result;
        }
        result.world = std::move(world_model);
        result.connected = connected_now;
    }
    return result;
}

}  // namespace skymapf::generator
