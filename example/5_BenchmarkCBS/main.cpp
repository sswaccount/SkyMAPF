#include <iostream>

#include "skymapf/algorithms/cbs/basic_cbs.hpp"
#include "skymapf/algorithms/prioritized/prioritized_planning.hpp"
#include "skymapf/benchmark/runner.hpp"
#include "skymapf/common/space.hpp"
#include "skymapf/generator/world_generator.hpp"
#include "skymapf/instance/model.hpp"
#include "skymapf/task/model.hpp"

namespace {

void print_run(const skymapf::benchmark::RunRecord& run) {
    std::cout << run.solver_info.name
              << "\tvalid=" << (run.valid_success() ? "yes" : "no")
              << "\tSOC=" << run.result.metrics.sum_of_costs
              << "\tmakespan=" << run.result.metrics.makespan
              << "\twall_ms=" << run.wall_time_ms << '\n';
}

}  // namespace

int main() {
    auto world = skymapf::generator::WorldGenerator::generate(
        skymapf::generator::WorldGenerationOptions(
            skymapf::common::SpaceSpec::make_2d(2, 2),
            0.0
        )
    );

    skymapf::task::TaskModel task(1, "two_agent_swap");
    task.add_agent_entry(1, skymapf::task::SequenceModel{{0, 1}});
    task.add_agent_entry(2, skymapf::task::SequenceModel{{1, 0}});
    auto instance = skymapf::instance::InstanceModel::create(
        1,
        std::move(world),
        std::move(task),
        "cbs_benchmark_demo"
    );

    const skymapf::solver::SolveOptions options{1000, 42, 1000};
    skymapf::algorithms::prioritized::PrioritizedPlanningSolver prioritized;
    skymapf::algorithms::cbs::BasicCBSSolver cbs;
    const auto prioritized_run = skymapf::benchmark::BenchmarkRunner::run(
        prioritized,
        instance,
        options
    );
    const auto cbs_run = skymapf::benchmark::BenchmarkRunner::run(cbs, instance, options);

    std::cout << "Two-agent swap benchmark\n";
    print_run(prioritized_run);
    print_run(cbs_run);
    return prioritized_run.valid_success() && cbs_run.valid_success() ? 0 : 1;
}
