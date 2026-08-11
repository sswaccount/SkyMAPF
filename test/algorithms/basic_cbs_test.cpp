#include <catch2/catch_test_macros.hpp>

#include "skymapf/algorithms/cbs/basic_cbs.hpp"
#include "skymapf/benchmark/runner.hpp"
#include "skymapf/common/space.hpp"
#include "skymapf/generator/world_generator.hpp"
#include "skymapf/instance/model.hpp"
#include "skymapf/search/conflict_detector.hpp"
#include "skymapf/task/model.hpp"

namespace {

skymapf::instance::InstanceModel make_swap_instance() {
    auto world = skymapf::generator::WorldGenerator::generate(
        skymapf::generator::WorldGenerationOptions(
            skymapf::common::SpaceSpec::make_2d(2, 2),
            0.0
        )
    );
    skymapf::task::TaskModel task(1, "swap");
    task.add_agent_entry(1, skymapf::task::SequenceModel{{0, 1}});
    task.add_agent_entry(2, skymapf::task::SequenceModel{{1, 0}});
    return skymapf::instance::InstanceModel::create(
        1,
        std::move(world),
        std::move(task),
        "swap"
    );
}

}  // namespace

TEST_CASE("basic CBS resolves an edge conflict", "[cbs][benchmark]") {
    const auto instance = make_swap_instance();
    skymapf::algorithms::cbs::BasicCBSSolver solver;

    const auto run = skymapf::benchmark::BenchmarkRunner::run(
        solver,
        instance,
        skymapf::solver::SolveOptions{1000, 42, 1000}
    );

    REQUIRE(run.solver_status == skymapf::solver::SolveStatus::Success);
    REQUIRE(run.valid_success());
    REQUIRE(run.result.plan.agent_paths.size() == 2);
    REQUIRE(run.result.metrics.sum_of_costs == 4);
    REQUIRE(run.result.metrics.makespan == 3);

    skymapf::search::Conflict conflict;
    REQUIRE_FALSE(skymapf::search::ConflictDetector::first_conflict(
        run.result.plan,
        conflict
    ));
}

TEST_CASE("basic CBS rejects unsupported extended task semantics", "[cbs]") {
    auto instance = make_swap_instance();
    auto extended = instance.task();
    extended.upsert_agent_entry(1, skymapf::task::SequenceModel{{0, 2, 1}});
    instance.set_task(std::move(extended));
    skymapf::algorithms::cbs::BasicCBSSolver solver;

    const auto result = solver.solve(instance, skymapf::solver::SolveOptions{});

    REQUIRE(result.status == skymapf::solver::SolveStatus::Unsupported);
}
