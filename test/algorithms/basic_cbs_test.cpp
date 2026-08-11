#include <catch2/catch_test_macros.hpp>

#include "skymapf/algorithms/cbs/basic_cbs.hpp"
#include "skymapf/algorithms/cbs/conflict_selector.hpp"
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
    REQUIRE(run.result.metrics.high_level_expanded_nodes > 0);
    REQUIRE(run.result.metrics.high_level_generated_nodes > 0);
    REQUIRE(run.result.metrics.low_level_expanded_nodes > 0);
    REQUIRE(run.result.metrics.low_level_generated_nodes > 0);
    REQUIRE(run.result.metrics.expanded_nodes ==
        run.result.metrics.high_level_expanded_nodes +
        run.result.metrics.low_level_expanded_nodes);
    REQUIRE(run.result.metrics.generated_nodes ==
        run.result.metrics.high_level_generated_nodes +
        run.result.metrics.low_level_generated_nodes);

    skymapf::search::Conflict conflict;
    REQUIRE_FALSE(skymapf::search::ConflictDetector::first_conflict(
        run.result.plan,
        conflict
    ));
}

TEST_CASE("CBS conflict selection can prioritize the earliest conflict", "[cbs][conflict]") {
    skymapf::solution::Plan plan;
    plan.agent_paths = {
        skymapf::solution::AgentPath{1, 0, {0, 1, 2}},
        skymapf::solution::AgentPath{2, 0, {3, 4, 2}},
        skymapf::solution::AgentPath{3, 0, {5, 1, 6}},
    };

    skymapf::search::Conflict first;
    REQUIRE(skymapf::algorithms::cbs::ConflictSelector::select(
        plan,
        skymapf::algorithms::cbs::ConflictSelectionStrategy::FirstDetected,
        first
    ));
    REQUIRE(first.time == 2);
    REQUIRE(first.a1 == 1);
    REQUIRE(first.a2 == 2);

    skymapf::search::Conflict earliest;
    REQUIRE(skymapf::algorithms::cbs::ConflictSelector::select(
        plan,
        skymapf::algorithms::cbs::ConflictSelectionStrategy::Earliest,
        earliest
    ));
    REQUIRE(earliest.time == 1);
    REQUIRE(earliest.a1 == 1);
    REQUIRE(earliest.a2 == 3);
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
