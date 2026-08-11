#include "skymapf/benchmark/runner.hpp"

#include <chrono>
#include <exception>
#include <string>

#include "skymapf/solution/metrics.hpp"
#include "skymapf/solution/validation.hpp"

namespace skymapf::benchmark {

RunRecord BenchmarkRunner::run(
    solver::ISolver& solver,
    const instance::InstanceModel& instance,
    const solver::SolveOptions& options
) {
    RunRecord record;
    record.instance_id = instance.id();
    record.solver_info = solver.info();
    record.random_seed = options.random_seed;

    const auto started = std::chrono::steady_clock::now();
    try {
        record.result = solver.solve(instance, options);
    } catch (const std::exception& ex) {
        record.result.status = solver::SolveStatus::Error;
        record.result.message = std::string{"solver threw an exception: "} + ex.what();
    } catch (...) {
        record.result.status = solver::SolveStatus::Error;
        record.result.message = "solver threw an unknown exception";
    }
    record.wall_time_ms = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - started
        ).count()
    );
    record.result.elapsed_ms = record.wall_time_ms;
    record.solver_status = record.result.status;

    if (!record.result.plan.empty() ||
        record.solver_status == solver::SolveStatus::Success) {
        record.validation_performed = true;
        record.validation = solution::SolutionValidator::validate(
            instance,
            record.result.plan
        );
    }

    if (record.solver_status == solver::SolveStatus::Success) {
        if (!record.validation.valid()) {
            record.result.status = solver::SolveStatus::Error;
            record.result.message = "benchmark validation rejected solver output";
            return record;
        }

        const auto canonical = solution::compute_metrics(record.result.plan);
        record.result.metrics.sum_of_costs = canonical.sum_of_costs;
        record.result.metrics.makespan = canonical.makespan;
    }

    return record;
}

}  // namespace skymapf::benchmark
