/**
 * @file reservation_table.hpp
 * @brief Declares reservations built from already planned paths.
 */
#pragma once

#include <cstddef>
#include <unordered_map>
#include <unordered_set>

#include "skymapf/common/ids.hpp"
#include "skymapf/solution/plan.hpp"

namespace skymapf::search {

class ReservationTable {
public:
    void clear() noexcept;
    void reserve_path(const solution::AgentPath& path, bool reserve_goal_forever = true);
    void reserve_plan(const solution::Plan& plan, bool reserve_goal_forever = true);

    bool reserved(common::CellIndex cell, common::TimeStep time) const;
    bool edge_reserved(
        common::CellIndex from,
        common::CellIndex to,
        common::TimeStep arrival_time
    ) const;

private:
    struct EdgeReservation {
        common::CellIndex from{0};
        common::CellIndex to{0};

        bool operator==(const EdgeReservation& other) const noexcept {
            return from == other.from && to == other.to;
        }
    };

    struct EdgeReservationHash {
        std::size_t operator()(const EdgeReservation& edge) const noexcept;
    };

    std::unordered_map<common::TimeStep, std::unordered_set<common::CellIndex>> vertices_;
    std::unordered_map<
        common::TimeStep,
        std::unordered_set<EdgeReservation, EdgeReservationHash>
    > edges_;
    std::unordered_map<common::CellIndex, common::TimeStep> persistent_goal_reservations_;
};

}  // namespace skymapf::search
