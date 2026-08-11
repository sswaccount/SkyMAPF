/**
 * @file runner.hpp
 * @brief Declares the framework-owned single-run benchmark pipeline.
 */
#pragma once

#include "run_record.hpp"
#include "skymapf/instance/model.hpp"
#include "skymapf/solver/i_solver.hpp"
#include "skymapf/solver/solve_options.hpp"

namespace skymapf::benchmark {

class BenchmarkRunner {
public:
    /**
     * @brief Runs one solver and independently evaluates its output.
     *
     * Wall time, validation, and standard solution-quality metrics in the
     * returned record are owned by the framework, not trusted from the solver.
     */
    static RunRecord run(
        solver::ISolver& solver,
        const instance::InstanceModel& instance,
        const solver::SolveOptions& options = {}
    );
};

}  // namespace skymapf::benchmark
