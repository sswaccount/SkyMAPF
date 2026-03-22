/**
 * @file route_sampling_strategy.cpp
 * @brief Implements default route sampling strategies for task generation.
 */
#include "skymapf/generator/strategy/route_sampling_strategy.hpp"

#include <random>
#include <vector>

#include "skymapf/generator/task_generator.hpp"

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

RouteSamplingResult RandomReachableRouteSamplingStrategy::sample(
    const RouteSamplingContext& context
) const {
    RouteSamplingResult result;
    task::SequenceModel seq;
    const auto& world_model = context.world;
    const auto start = context.start_cell;
    if (!world_model.is_valid_index(start) || !world_model.is_walkable(start)) {
        result.error_message = "Invalid or non-walkable start cell.";
        return result;
    }

    auto walkable = collect_walkable_cells(world_model);
    if (walkable.empty()) {
        result.error_message = "World has no walkable cell.";
        return result;
    }

    std::mt19937_64 rng(context.agent_seed);
    std::uniform_int_distribution<std::size_t> pick(0, walkable.size() - 1);
    const auto max_size = context.options.max_sequence_size;
    std::uniform_int_distribution<std::size_t> seq_len_pick(2, max_size);
    const auto sequence_size = seq_len_pick(rng);

    seq.checkpoints.reserve(sequence_size);
    seq.checkpoints.push_back(start);
    for (std::size_t i = 0; i + 1 < sequence_size; ++i) {
        seq.checkpoints.push_back(walkable[pick(rng)]);
    }
    result.success = true;
    result.sequence = std::move(seq);
    return result;
}

}  // namespace skymapf::generator
