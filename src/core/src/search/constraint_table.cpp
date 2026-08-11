#include "skymapf/search/constraint_table.hpp"

namespace skymapf::search {

void ConstraintTable::add(Constraint constraint) {
    constraints_.push_back(constraint);
}

bool ConstraintTable::violates_vertex(
    common::CellIndex cell,
    common::TimeStep time
) const noexcept {
    for (const auto& constraint : constraints_) {
        if ((constraint.kind == ConstraintKind::Vertex ||
             constraint.kind == ConstraintKind::Range) &&
            constraint.from == cell &&
            time >= constraint.time_min &&
            time <= constraint.time_max) {
            return true;
        }
    }
    return false;
}

bool ConstraintTable::violates_edge(
    common::CellIndex from,
    common::CellIndex to,
    common::TimeStep arrival_time
) const noexcept {
    for (const auto& constraint : constraints_) {
        if (constraint.kind == ConstraintKind::Edge &&
            constraint.from == from &&
            constraint.to == to &&
            arrival_time >= constraint.time_min &&
            arrival_time <= constraint.time_max) {
            return true;
        }
    }
    return false;
}

bool ConstraintTable::has_future_vertex_constraint(
    common::CellIndex cell,
    common::TimeStep time
) const noexcept {
    for (const auto& constraint : constraints_) {
        if ((constraint.kind == ConstraintKind::Vertex ||
             constraint.kind == ConstraintKind::Range) &&
            constraint.from == cell &&
            constraint.time_max >= time) {
            return true;
        }
    }
    return false;
}

}  // namespace skymapf::search
