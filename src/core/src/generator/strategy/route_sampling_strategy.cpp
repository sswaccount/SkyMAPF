/**
 * @file route_sampling_strategy.cpp
 * @brief Implements default route sampling strategies for task generation.
 */
#include "skymapf/generator/strategy/route_sampling_strategy.hpp"

#include <algorithm>
#include <random>
#include <vector>

#include "skymapf/utils/random_tool.hpp"

namespace skymapf::generator {

void RandomReachableRouteSamplingStrategy::apply(
    const world::WorldModel& world,
    task::TaskModel& task,
    const RouteSamplingContext& context
) const {
    const auto walkable = world.walkable_cells();

    auto& rng = utils::RandomTool::instance().generator_rng();
    std::uniform_int_distribution<std::size_t> pick_walkable(
        0,
        walkable.size() - 1
    );
    std::uniform_int_distribution<common::TimeStep> pick_start_time(
        0,
        context.max_start_time
    );

    const auto max_seq_len = std::max<std::uint32_t>(2, context.max_sequence_size);
    std::uniform_int_distribution<std::uint32_t> pick_seq_len(2, max_seq_len);

    const auto next_unique_agent_id = [&task, &rng]() {
        while (true) {
            const auto candidate = static_cast<common::AgentId>(rng());
            if (!task.has_agent(candidate)) {
                return candidate;
            }
        }
    };

    for (std::size_t i = 0; i < context.agent_count; ++i) {
        const auto agent_id = next_unique_agent_id();
        const auto start = walkable[pick_walkable(rng)];
        const auto seq_len = pick_seq_len(rng);

        task::SequenceModel seq;
        seq.checkpoints.reserve(static_cast<std::size_t>(seq_len));
        seq.checkpoints.push_back(start);

        auto current = start;
        for (std::uint32_t step = 1; step < seq_len; ++step) {
            auto neighbors = world.neighbors(current);
            if (neighbors.empty()) {
                // Keep sequence reachable even in degenerate connectivity layouts.
                seq.checkpoints.push_back(current);
                continue;
            }
            std::uniform_int_distribution<std::size_t> pick_neighbor(
                0,
                neighbors.size() - 1
            );
            current = neighbors[pick_neighbor(rng)];
            seq.checkpoints.push_back(current);
        }

        task.upsert_agent_entry(
            agent_id,
            std::move(seq),
            pick_start_time(rng),
            context.goal_arrival_behavior
        );
    }
}

}  // namespace skymapf::generator
