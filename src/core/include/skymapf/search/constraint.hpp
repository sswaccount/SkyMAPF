/**
 * @file constraint.hpp
 * @brief Defines generic time-space constraints used by MAPF solvers.
 */
#pragma once

#include "skymapf/common/ids.hpp"

namespace skymapf::search {

enum class ConstraintKind {
    Vertex,
    Edge,
    Range,
    Barrier,
    LengthUpperBound,
    LengthLowerBound,
    PositiveVertex,
    PositiveEdge,
    PositiveRange,
    PositiveBarrier,
};

struct Constraint {
    common::AgentId agent_id{0};
    ConstraintKind kind{ConstraintKind::Vertex};
    common::CellIndex from{0};
    common::CellIndex to{0};
    common::TimeStep time_min{0};
    common::TimeStep time_max{0};

    static Constraint vertex(
        common::AgentId agent_id,
        common::CellIndex cell,
        common::TimeStep time
    );

    static Constraint edge(
        common::AgentId agent_id,
        common::CellIndex from,
        common::CellIndex to,
        common::TimeStep arrival_time
    );
};

}  // namespace skymapf::search
