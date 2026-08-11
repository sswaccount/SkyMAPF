#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "skymapf/algorithms/cbs/basic_cbs.hpp"
#include "skymapf/algorithms/prioritized/prioritized_planning.hpp"
#include "skymapf/benchmark/runner.hpp"
#include "skymapf/io/movingai_io.hpp"

namespace {

std::filesystem::path fixture_path(const char* filename) {
    return std::filesystem::path(__FILE__).parent_path().parent_path() /
        "data" / "movingai" / filename;
}

}  // namespace

TEST_CASE("MovingAI map and scenario are imported with standard semantics", "[io][movingai]") {
    std::string error;
    const auto world = skymapf::io::MovingAIIO::read_map(
        fixture_path("tiny.map"),
        41,
        &error
    );
    REQUIRE(world.has_value());
    REQUIRE(error.empty());
    REQUIRE(world->id() == 41);
    REQUIRE(world->space_spec().cols() == 2);
    REQUIRE(world->space_spec().rows() == 2);
    REQUIRE(world->walkable_cells().size() == 4);

    const auto neighbors = world->neighbors(0);
    REQUIRE(std::find(neighbors.begin(), neighbors.end(), 1) != neighbors.end());
    REQUIRE(std::find(neighbors.begin(), neighbors.end(), 2) != neighbors.end());
    REQUIRE(std::find(neighbors.begin(), neighbors.end(), 3) == neighbors.end());

    const auto entries = skymapf::io::MovingAIIO::read_scenario(
        fixture_path("tiny.scen"),
        &error
    );
    REQUIRE(entries.has_value());
    REQUIRE(entries->size() == 2);
    REQUIRE(entries->front().map_name == "tiny.map");
    REQUIRE(entries->front().start_x == 0);
    REQUIRE(entries->front().goal_x == 1);
    REQUIRE(entries->front().reference_distance == 1.0);
}

TEST_CASE("MovingAI prefixes run through both baseline solvers", "[io][movingai][benchmark]") {
    std::string error;
    skymapf::io::MovingAIImportOptions options;
    options.world_id = 41;
    options.first_task_id = 51;
    options.first_instance_id = 61;
    const auto instances = skymapf::io::MovingAIIO::read_prefix_instances(
        fixture_path("tiny.map"),
        fixture_path("tiny.scen"),
        {1, 2},
        options,
        &error
    );

    REQUIRE(instances.has_value());
    REQUIRE(error.empty());
    REQUIRE(instances->size() == 2);
    REQUIRE((*instances)[0].id() == 61);
    REQUIRE((*instances)[0].task().agent_count() == 1);
    REQUIRE((*instances)[1].id() == 62);
    REQUIRE((*instances)[1].task().id() == 52);
    REQUIRE((*instances)[1].task().agent_count() == 2);

    skymapf::algorithms::prioritized::PrioritizedPlanningSolver prioritized;
    skymapf::algorithms::cbs::BasicCBSSolver cbs;
    const auto solve_options = skymapf::solver::SolveOptions{1000, 42, 1000};
    const auto prioritized_run = skymapf::benchmark::BenchmarkRunner::run(
        prioritized,
        (*instances)[1],
        solve_options
    );
    const auto cbs_run = skymapf::benchmark::BenchmarkRunner::run(
        cbs,
        (*instances)[1],
        solve_options
    );

    REQUIRE(prioritized_run.valid_success());
    REQUIRE(cbs_run.valid_success());
    REQUIRE(prioritized_run.result.metrics.sum_of_costs == 4);
    REQUIRE(cbs_run.result.metrics.sum_of_costs == 4);
    REQUIRE(prioritized_run.result.metrics.makespan == 3);
    REQUIRE(cbs_run.result.metrics.makespan == 3);
}

TEST_CASE("MovingAI prefix conversion rejects incompatible scenarios", "[io][movingai]") {
    std::string error;
    const auto world = skymapf::io::MovingAIIO::read_map(
        fixture_path("tiny.map"),
        1,
        &error
    );
    const auto entries = skymapf::io::MovingAIIO::read_scenario(
        fixture_path("tiny.scen"),
        &error
    );
    REQUIRE(world.has_value());
    REQUIRE(entries.has_value());

    auto incompatible_entries = *entries;
    incompatible_entries.front().map_width = 3;
    const auto instance = skymapf::io::MovingAIIO::make_prefix_instance(
        *world,
        incompatible_entries,
        1,
        1,
        1,
        {},
        &error
    );

    REQUIRE_FALSE(instance.has_value());
    REQUIRE(error == "MovingAI scenario dimensions do not match map");
}
