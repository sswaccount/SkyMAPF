#include "skymapf/search/reservation_table.hpp"

#include <functional>

namespace skymapf::search {

std::size_t ReservationTable::EdgeReservationHash::operator()(
    const EdgeReservation& edge
) const noexcept {
    return std::hash<common::CellIndex>{}(edge.from) ^
           (std::hash<common::CellIndex>{}(edge.to) << 1U);
}

void ReservationTable::clear() noexcept {
    vertices_.clear();
    edges_.clear();
    persistent_goal_reservations_.clear();
}

void ReservationTable::reserve_path(
    const solution::AgentPath& path,
    bool reserve_goal_forever
) {
    if (path.cells.empty()) {
        return;
    }
    for (std::size_t i = 0; i < path.cells.size(); ++i) {
        const auto time = static_cast<common::TimeStep>(path.start_time + i);
        vertices_[time].insert(path.cells[i]);
        if (i > 0) {
            const auto prev = path.cells[i - 1];
            const auto curr = path.cells[i];
            edges_[time].insert(EdgeReservation{prev, curr});
        }
    }
    if (reserve_goal_forever) {
        persistent_goal_reservations_[path.cells.back()] = path.end_time();
    }
}

void ReservationTable::reserve_plan(
    const solution::Plan& plan,
    bool reserve_goal_forever
) {
    for (const auto& path : plan.agent_paths) {
        reserve_path(path, reserve_goal_forever);
    }
}

bool ReservationTable::reserved(
    common::CellIndex cell,
    common::TimeStep time
) const {
    const auto vertex_it = vertices_.find(time);
    if (vertex_it != vertices_.end() && vertex_it->second.count(cell) > 0) {
        return true;
    }
    const auto goal_it = persistent_goal_reservations_.find(cell);
    return goal_it != persistent_goal_reservations_.end() && time >= goal_it->second;
}

bool ReservationTable::edge_reserved(
    common::CellIndex from,
    common::CellIndex to,
    common::TimeStep arrival_time
) const {
    const auto edge_it = edges_.find(arrival_time);
    if (edge_it == edges_.end()) {
        return false;
    }
    return edge_it->second.count(EdgeReservation{to, from}) > 0 ||
           edge_it->second.count(EdgeReservation{from, to}) > 0;
}

}  // namespace skymapf::search
