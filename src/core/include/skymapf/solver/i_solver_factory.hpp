#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "i_solver.hpp"

namespace skymapf::solver {

// Factory interface used to discover and construct solvers by name.
class ISolverFactory {
public:
    virtual ~ISolverFactory() = default;

    // Returns all registered solver names, e.g. {"cbs", "ilp_cpsat"}.
    virtual std::vector<std::string> registered_solver_names() const = 0;

    // Checks whether a solver is available in this factory.
    virtual bool is_registered(std::string_view solver_name) const = 0;

    // Creates a solver instance by name. Returns nullptr when unavailable.
    virtual std::unique_ptr<ISolver> create(std::string_view solver_name) const = 0;
};

}  // namespace skymapf::solver
