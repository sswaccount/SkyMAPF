/**
 * @file edge_generation_strategy.cpp
 * @brief Implements built-in edge generation strategies.
 */
#include "skymapf/generator/strategy/edge_generation_strategy.hpp"

#include "skymapf/world/connectivity.hpp"

namespace skymapf::generator {

void BidirectionalGeometricEdgeGenerationStrategy::apply(world::WorldModel& world_model) const {
    const auto& spec = world_model.space_spec();
    const auto total = world_model.cell_count();
    world_model.clear_edges();

    for (common::CellIndex from = 0; from < total; ++from) {
        for (const auto to : world::geometric_neighbors(spec, from)) {
            if (from < to) {
                world_model.add_bidirectional_edge(from, to);
            }
        }
    }
}

}  // namespace skymapf::generator
