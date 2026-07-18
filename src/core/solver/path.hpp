#pragma once

#include <vector>

namespace skymapf {

struct PathEntry {
    int location_id = -1;
    int timestep = 0;
};

using Path = std::vector<PathEntry>;

} // namespace skymapf
