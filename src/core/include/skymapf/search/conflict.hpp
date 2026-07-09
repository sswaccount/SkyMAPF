/**
 * @file conflict.hpp
 * @brief Defines generic MAPF conflict models.
 */
#pragma once

#include "skymapf/common/ids.hpp"

namespace skymapf::search {

enum class ConflictKind {
    Vertex,
    Edge,
    Target,
    Rectangle,
    Corridor,
    Mutex,
};

enum class ConflictPriority {
    Cardinal,
    SemiCardinal,
    NonCardinal,
    Unknown,
};

struct Conflict {
    common::AgentId a1{0};
    common::AgentId a2{0};
    ConflictKind kind{ConflictKind::Vertex};
    ConflictPriority priority{ConflictPriority::Unknown};
    common::TimeStep time{0};
    common::CellIndex cell{0};
    common::CellIndex a1_from{0};
    common::CellIndex a1_to{0};
    common::CellIndex a2_from{0};
    common::CellIndex a2_to{0};
};

}  // namespace skymapf::search
