#include "skymapf/algorithms/prioritized/prioritized_planning.hpp"
#include "skymapf/common/space.hpp"
#include "skymapf/generator/world_generator.hpp"
#include "skymapf/instance/model.hpp"
#include "skymapf/task/model.hpp"

#include <iostream>

int main() {
    auto world = skymapf::generator::WorldGenerator::generate(
        skymapf::generator::WorldGenerationOptions(
            skymapf::common::SpaceSpec::make_2d(4, 1),
            0.0
        )
    );

    skymapf::task::TaskModel task;
    task.add_agent_entry(
        1,
        skymapf::task::SequenceModel{{0, 3}},
        0,
        skymapf::task::GoalArrivalBehavior::StayAtGoal
    );

    auto instance = skymapf::instance::InstanceModel::create(
        1,
        std::move(world),
        std::move(task),
        "prioritized_planning_demo"
    );

    skymapf::algorithms::prioritized::PrioritizedPlanningSolver solver;
    auto result = solver.solve(instance, skymapf::solver::SolveOptions{});

    if (result.status != skymapf::solver::SolveStatus::Success) {
        std::cerr << "Solve failed: " << result.message << '\n';
        return 1;
    }

    std::cout << "Solver: " << solver.info().name << '\n';
    std::cout << "Agent paths: " << result.plan.agent_paths.size() << '\n';
    std::cout << "Sum of costs: " << result.metrics.sum_of_costs << '\n';
    std::cout << "Makespan: " << result.metrics.makespan << '\n';
    return 0;
}
