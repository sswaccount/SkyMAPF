#include "skymapf/search/constraint.hpp"

namespace skymapf::search {

Constraint Constraint::vertex(
    common::AgentId agent_id,
    common::CellIndex cell,
    common::TimeStep time
) {
    return Constraint{agent_id, ConstraintKind::Vertex, cell, cell, time, time};
}

Constraint Constraint::edge(
    common::AgentId agent_id,
    common::CellIndex from,
    common::CellIndex to,
    common::TimeStep arrival_time
) {
    return Constraint{agent_id, ConstraintKind::Edge, from, to, arrival_time, arrival_time};
}

}  // namespace skymapf::search
