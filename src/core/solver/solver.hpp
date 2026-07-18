#pragma once

#include "solution.hpp"

namespace skymapf {

class MAPFInstance;

class Solver {
public:
    virtual ~Solver() = default;
    virtual Solution solve(const MAPFInstance& instance) = 0;
};

} // namespace skymapf
