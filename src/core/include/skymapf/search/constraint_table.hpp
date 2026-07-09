/**
 * @file constraint_table.hpp
 * @brief Declares per-agent constraint lookup helpers.
 */
#pragma once

#include <vector>

#include "constraint.hpp"

namespace skymapf::search {

class ConstraintTable {
public:
    void add(Constraint constraint);
    void clear() noexcept { constraints_.clear(); }

    bool violates_vertex(common::CellIndex cell, common::TimeStep time) const noexcept;
    bool violates_edge(
        common::CellIndex from,
        common::CellIndex to,
        common::TimeStep arrival_time
    ) const noexcept;

    const std::vector<Constraint>& constraints() const noexcept { return constraints_; }

private:
    std::vector<Constraint> constraints_;
};

}  // namespace skymapf::search
