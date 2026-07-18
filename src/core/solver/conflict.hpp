#pragma once

namespace skymapf {

enum class ConflictType {
    Vertex,
    Edge
};

struct Conflict {
    int agent1 = -1;
    int agent2 = -1;
    int timestep = 0;
    ConflictType type = ConflictType::Vertex;

    int location1 = -1;
    int location2 = -1;
};

} // namespace skymapf
