#include "skymapf/generator/task_generator.hpp"
#include "skymapf/generator/scenario_generator.hpp"
#include "skymapf/generator/world_generator.hpp"

using namespace std;
using namespace skymapf::generator;

int main() {
    auto world_model = WorldGenerator::generate(
        WorldGenerationRequest()
    );
    vector<TaskGenerationRequest> task_requests;
    task_requests.push_back(TaskGenerationRequest{
        1,
        {1, 2, 3},
        2,
        0,
        "task_a",
        2026,
        nullptr
    });
    task_requests.push_back(TaskGenerationRequest{
        2,
        {4, 5},
        2,
        10,
        "task_b",
        3036,
        nullptr
    });
    auto task_models = TaskGenerator::generate_family(
        world_model,
        task_requests
    );

    auto scenario_model = skymapf::scenario::ScenarioModel::create(
        1,
        world_model,
        std::move(task_models),
        "demo_scenario"
    );

    auto instances = ScenarioGenerator::generate(
        scenario_model,
        InstanceGenerationOptions{1}
    );
    if (instances.empty()) {
        return 1;
    }

    return 0;
}
