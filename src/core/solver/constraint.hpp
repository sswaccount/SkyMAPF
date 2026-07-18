#pragma once

namespace skymapf {

enum class ConstraintType {
    Vertex,
    Edge
};

struct Constraint {
    int agent = -1;
    int timestep = 0;
    ConstraintType type = ConstraintType::Vertex;

    int location1 = -1;
    int location2 = -1;
};

} // namespace skymapf
