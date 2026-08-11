#include <catch2/catch_test_macros.hpp>

#include "skymapf/benchmark/runner.hpp"
#include "skymapf/common/space.hpp"
#include "skymapf/generator/world_generator.hpp"
#include "skymapf/instance/model.hpp"
#include "skymapf/solver/i_solver.hpp"
#include "skymapf/task/model.hpp"

namespace {

class InvalidSolver final : public skymapf::solver::ISolver {
public:
    skymapf::solver::SolverInfo info() const override {
        return {"invalid_test_solver", skymapf::solver::SolverFamily::Unknown, "test", {}, {}};
    }

    skymapf::solver::SolveResult solve(
        const skymapf::solver::SolveInstance&,
        const skymapf::solver::SolveOptions&
    ) override {
        skymapf::solver::SolveResult result;
        result.status = skymapf::solver::SolveStatus::Success;
        result.plan.agent_paths.push_back({1, 0, {0, 1}});
        result.metrics.sum_of_costs = 999;
        return result;
    }
};

skymapf::instance::InstanceModel make_instance() {
    auto world = skymapf::generator::WorldGenerator::generate(
        skymapf::generator::WorldGenerationOptions(
            skymapf::common::SpaceSpec::make_2d(2, 1),
            0.0
        )
    );
    skymapf::task::TaskModel task(1, "runner");
    task.add_agent_entry(1, skymapf::task::SequenceModel{{0, 1}});
    task.add_agent_entry(2, skymapf::task::SequenceModel{{1, 0}});
    return skymapf::instance::InstanceModel::create(
        7,
        std::move(world),
        std::move(task),
        "runner"
    );
}

}  // namespace

TEST_CASE("benchmark runner independently rejects invalid solver output", "[benchmark]") {
    auto instance = make_instance();
    InvalidSolver solver;

    const auto run = skymapf::benchmark::BenchmarkRunner::run(solver, instance);

    REQUIRE(run.instance_id == 7);
    REQUIRE(run.solver_status == skymapf::solver::SolveStatus::Success);
    REQUIRE(run.result.status == skymapf::solver::SolveStatus::Error);
    REQUIRE_FALSE(run.validation.valid());
    REQUIRE_FALSE(run.valid_success());
}

TEST_CASE("benchmark validator rejects paths for agents outside the instance", "[benchmark]") {
    auto instance = make_instance();
    InvalidSolver solver;
    auto result = solver.solve(
        skymapf::solver::SolveInstance{instance},
        skymapf::solver::SolveOptions{}
    );
    result.plan.agent_paths.push_back({99, 0, {0, 1}});

    const auto validation = skymapf::solution::SolutionValidator::validate(
        instance,
        result.plan
    );

    REQUIRE_FALSE(validation.valid());
}
