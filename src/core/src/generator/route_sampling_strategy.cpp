/**
 * @file route_sampling_strategy.cpp
 * @brief Implements default route sampling strategies for task generation.
 */
#include "skymapf/generator/route_sampling_strategy.hpp"

#include <random>
#include <vector>

namespace skymapf::generator {

namespace {

std::vector<common::CellIndex> collect_walkable_cells(const world::WorldModel& world_model) {
    std::vector<common::CellIndex> cells;
    cells.reserve(static_cast<std::size_t>(world_model.cell_count()));
    for (common::CellIndex i = 0; i < world_model.cell_count(); ++i) {
        if (world_model.is_walkable(i)) {
            cells.push_back(i);
        }
    }
    return cells;
}

}  // namespace

task::SequenceModel RandomReachableRouteSamplingStrategy::sample(
    const world::WorldModel& world_model,
    common::CellIndex start,
    std::size_t waypoint_count,
    std::uint64_t random_seed
) const {
    task::SequenceModel seq;
    if (!world_model.is_valid_index(start) || !world_model.is_walkable(start)) {
        return seq;
    }

    auto walkable = collect_walkable_cells(world_model);
    if (walkable.empty()) {
        return seq;
    }

    std::mt19937_64 rng(random_seed);
    std::uniform_int_distribution<std::size_t> pick(0, walkable.size() - 1);

    seq.checkpoints.reserve(waypoint_count + 2);
    seq.checkpoints.push_back(start);
    for (std::size_t i = 0; i < waypoint_count; ++i) {
        seq.checkpoints.push_back(walkable[pick(rng)]);
    }
    seq.checkpoints.push_back(walkable[pick(rng)]);
    return seq;
}

}  // namespace skymapf::generator
