/**
 * @file i_solver.hpp
 * @brief Declares the base solver interface and shared solve data models.
 */
#pragma once

#include "../instance/model.hpp"
#include "model/solver_info.hpp"
#include "solve_options.hpp"
#include "solve_result.hpp"

namespace skymapf::solver {

/// Input bundle for one solver invocation.
struct SolveInstance {
    instance::InstanceModel instance;
};

/**
 * @brief Abstract interface for all planner implementations.
 */
class ISolver {
public:
    virtual ~ISolver() = default;

    /// Returns static solver metadata.
    virtual SolverInfo info() const = 0;
    /**
     * @brief Solves one planning instance under given runtime options.
     *
     * @param instance Planning inputs.
     * @param options Runtime solve options.
     * @return Solve status, optional plan, and diagnostics.
     */
    virtual SolveResult solve(
        const SolveInstance& instance,
        const SolveOptions& options
    ) = 0;

    /// Convenience overload for callers that already hold an InstanceModel.
    virtual SolveResult solve(
        const instance::InstanceModel& instance,
        const SolveOptions& options
    ) {
        return solve(SolveInstance{instance}, options);
    }
};

}  // namespace skymapf::solver
