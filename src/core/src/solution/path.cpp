#include "skymapf/solution/path.hpp"

namespace skymapf::solution {

common::TimeStep AgentPath::end_time() const noexcept {
    if (cells.empty()) {
        return start_time;
    }
    return static_cast<common::TimeStep>(start_time + cells.size() - 1);
}

common::CellIndex AgentPath::location_at(common::TimeStep time) const noexcept {
    if (cells.empty()) {
        return 0;
    }
    if (time <= start_time) {
        return cells.front();
    }
    const auto offset = static_cast<std::size_t>(time - start_time);
    if (offset >= cells.size()) {
        return cells.back();
    }
    return cells[offset];
}

}  // namespace skymapf::solution
