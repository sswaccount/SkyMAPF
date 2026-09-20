#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "skymapf/algorithms/cbs/basic_cbs.hpp"
#include "skymapf/algorithms/prioritized/prioritized_planning.hpp"
#include "skymapf/benchmark/runner.hpp"
#include "skymapf/common/space.hpp"
#include "skymapf/common/index.hpp"
#include "skymapf/generator/world_generator.hpp"
#include "skymapf/instance/model.hpp"
#include "skymapf/task/model.hpp"

namespace {

void print_run(
    const std::string& benchmark,
    std::size_t agents,
    const skymapf::instance::InstanceModel& instance,
    const skymapf::benchmark::RunRecord& run
) {
    nlohmann::ordered_json solution;
    const auto shape = instance.world().space_spec().shape_2d();
    solution["name"] = benchmark;
    solution["rows"] = shape.rows;
    solution["cols"] = shape.cols;
    solution["obstacles"] = nlohmann::ordered_json::array();
    for (skymapf::common::CellIndex cell = 0; cell < instance.world().cell_count(); ++cell) {
        if (!instance.world().is_walkable(cell)) {
            const auto coord = skymapf::common::to_coord_2d(cell, shape);
            solution["obstacles"].push_back({coord.x, coord.y});
        }
    }
    solution["agents"] = nlohmann::ordered_json::array();
    std::size_t total_steps = 0;
    for (const auto& path : run.result.plan.agent_paths) {
        nlohmann::ordered_json agent;
        agent["id"] = path.agent_id;
        agent["path"] = nlohmann::ordered_json::array();
        for (const auto cell : path.cells) {
            const auto coord = skymapf::common::to_coord_2d(cell, shape);
            agent["path"].push_back({coord.x, coord.y});
        }
        if (!path.cells.empty()) {
            agent["start"] = agent["path"].front();
            agent["goal"] = agent["path"].back();
            total_steps = std::max(total_steps, path.cells.size() - 1);
        }
        solution["agents"].push_back(std::move(agent));
    }
    solution["totalSteps"] = total_steps;
    std::cout << "SKYMAPF_RESULT"
              << "\tbenchmark=" << benchmark
              << "\tsolver=" << run.solver_info.name
              << "\tagents=" << agents
              << "\tvalid=" << (run.valid_success() ? "yes" : "no")
              << "\tsoc=" << run.result.metrics.sum_of_costs
              << "\tmakespan=" << run.result.metrics.makespan
              << "\twall_ms=" << run.wall_time_ms
              << "\texpanded=" << run.result.metrics.expanded_nodes
              << "\tgenerated=" << run.result.metrics.generated_nodes
              << "\tsolution=" << solution.dump() << '\n';
}

struct BenchmarkCase {
    std::string name;
    skymapf::common::SpaceSpec space;
    std::vector<std::pair<skymapf::common::CellIndex, skymapf::common::CellIndex>> routes;
    std::vector<skymapf::common::CellIndex> obstacles{};
};

}  // namespace

int main() {
    const std::vector<BenchmarkCase> cases{
        {"swap-2x2", skymapf::common::SpaceSpec::make_2d(2, 2), {{0, 1}, {1, 0}}},
        {"cross-3x3", skymapf::common::SpaceSpec::make_2d(3, 3), {{3, 5}, {1, 7}}},
        {"rotation-3x3", skymapf::common::SpaceSpec::make_2d(3, 3), {{0, 2}, {2, 8}, {8, 6}}},
        {"crossing-5x5", skymapf::common::SpaceSpec::make_2d(5, 5),
         {{10, 14}, {2, 22}, {5, 9}, {19, 15}, {20, 4}}},
        {"warehouse-10x10", skymapf::common::SpaceSpec::make_2d(10, 10),
         {{0, 99}, {9, 90}, {90, 9}, {99, 0}, {40, 49}, {59, 50}, {2, 97}, {7, 92}},
         {14, 24, 34, 64, 74, 84, 52, 53, 56, 57}},
    };
    const skymapf::solver::SolveOptions options{5000, 42, 100000};
    skymapf::common::InstanceId instance_id = 1;

    for (const auto& benchmark : cases) {
        auto world = skymapf::generator::WorldGenerator::generate(
            skymapf::generator::WorldGenerationOptions(benchmark.space, 0.0)
        );
        for (const auto obstacle : benchmark.obstacles) {
            world.set_walkable(obstacle, false);
        }
        skymapf::task::TaskModel task(instance_id, benchmark.name);
        skymapf::common::AgentId agent_id = 1;
        for (const auto& [start, goal] : benchmark.routes) {
            task.add_agent_entry(agent_id++, skymapf::task::SequenceModel{{start, goal}});
        }
        auto instance = skymapf::instance::InstanceModel::create(
            instance_id++, std::move(world), std::move(task), benchmark.name
        );
        skymapf::algorithms::prioritized::PrioritizedPlanningSolver prioritized;
        skymapf::algorithms::cbs::BasicCBSSolver cbs;
        const auto prioritized_run = skymapf::benchmark::BenchmarkRunner::run(
            prioritized, instance, options
        );
        const auto cbs_run = skymapf::benchmark::BenchmarkRunner::run(cbs, instance, options);
        print_run(benchmark.name, benchmark.routes.size(), instance, prioritized_run);
        print_run(benchmark.name, benchmark.routes.size(), instance, cbs_run);
    }
    return 0;
}
