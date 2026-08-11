#include <chrono>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "skymapf/benchmark/runner.hpp"
#include "skymapf/common/space.hpp"
#include "skymapf/generator/world_generator.hpp"
#include "skymapf/instance/model.hpp"
#include "skymapf/solver/external_process_solver.hpp"
#include "skymapf/task/model.hpp"

namespace {

skymapf::instance::InstanceModel make_swap_instance() {
    auto world = skymapf::generator::WorldGenerator::generate(
        skymapf::generator::WorldGenerationOptions(
            skymapf::common::SpaceSpec::make_2d(2, 2),
            0.0
        )
    );
    skymapf::task::TaskModel task(1, "external-swap");
    task.add_agent_entry(1, skymapf::task::SequenceModel{{0, 1}});
    task.add_agent_entry(2, skymapf::task::SequenceModel{{1, 0}});
    return skymapf::instance::InstanceModel::create(
        77,
        std::move(world),
        std::move(task),
        "external-swap"
    );
}

skymapf::solver::ExternalProcessSolver make_solver(const std::string& mode) {
    skymapf::solver::ExternalProcessSolverConfig config;
    config.executable = SKYMAPF_EXTERNAL_SOLVER_FIXTURE;
    config.arguments = {mode};
    config.solver_info.name = "external_fixture";
    config.solver_info.version = "1.0";
    return skymapf::solver::ExternalProcessSolver(std::move(config));
}

}  // namespace

TEST_CASE("external process solver completes the benchmark pipeline", "[solver][external]") {
    const auto instance = make_swap_instance();
    auto solver = make_solver("success");
    const auto run = skymapf::benchmark::BenchmarkRunner::run(
        solver,
        instance,
        skymapf::solver::SolveOptions{5000, 42, 1000}
    );

    REQUIRE(run.solver_status == skymapf::solver::SolveStatus::Success);
    REQUIRE(run.valid_success());
    REQUIRE(run.result.metrics.sum_of_costs == 4);
    REQUIRE(run.result.metrics.makespan == 3);
    REQUIRE(run.result.metrics.expanded_nodes == 7);
    REQUIRE(run.result.metrics.generated_nodes == 11);
}

#if defined(SKYMAPF_PYTHON_EXECUTABLE) && defined(SKYMAPF_PYTHON_SOLVER_FIXTURE)
TEST_CASE("Python SDK solver completes the C++ benchmark pipeline", "[solver][external][python]") {
    skymapf::solver::ExternalProcessSolverConfig config;
    config.executable = SKYMAPF_PYTHON_EXECUTABLE;
    config.arguments = {SKYMAPF_PYTHON_SOLVER_FIXTURE};
    config.solver_info.name = "python_fixture";
    config.solver_info.version = "1.0";
    skymapf::solver::ExternalProcessSolver solver(std::move(config));

    const auto run = skymapf::benchmark::BenchmarkRunner::run(
        solver,
        make_swap_instance(),
        skymapf::solver::SolveOptions{5000, 42, 1000}
    );

    REQUIRE(run.solver_status == skymapf::solver::SolveStatus::Success);
    REQUIRE(run.valid_success());
    REQUIRE(run.result.metrics.sum_of_costs == 4);
    REQUIRE(run.result.metrics.makespan == 3);
    REQUIRE(run.result.metrics.expanded_nodes == 13);
    REQUIRE(run.result.metrics.generated_nodes == 17);
}
#endif

TEST_CASE("external process solver enforces a hard timeout", "[solver][external]") {
    const auto instance = make_swap_instance();
    auto solver = make_solver("sleep");
    const auto started = std::chrono::steady_clock::now();
    const auto run = skymapf::benchmark::BenchmarkRunner::run(
        solver,
        instance,
        skymapf::solver::SolveOptions{20, 42, 0}
    );
    const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - started
    ).count();

    REQUIRE(run.solver_status == skymapf::solver::SolveStatus::Timeout);
    REQUIRE(run.result.status == skymapf::solver::SolveStatus::Timeout);
    REQUIRE(elapsed_ms < 1000);
}

TEST_CASE("external process solver maps malformed output to error", "[solver][external]") {
    const auto instance = make_swap_instance();
    auto solver = make_solver("invalid-json");
    const auto run = skymapf::benchmark::BenchmarkRunner::run(solver, instance);

    REQUIRE(run.solver_status == skymapf::solver::SolveStatus::Error);
    REQUIRE(run.result.status == skymapf::solver::SolveStatus::Error);
}
