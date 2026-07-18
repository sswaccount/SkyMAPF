#pragma once

#include <vector>

#include "path.hpp"

namespace skymapf {

struct Solution {
    bool success = false;
    double runtime = 0.0;
    int sum_of_costs = 0;
    int makespan = 0;
    std::vector<Path> paths;
};

} // namespace skymapf
