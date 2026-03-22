#include "skymapf/generator/task_generator.hpp"
#include "skymapf/generator/scenario_generator.hpp"
#include "skymapf/generator/world_generator.hpp"
#include <iostream>
#include <vector>

using namespace std;
using namespace skymapf::generator;
using namespace skymapf::scenario;

int main() {
    auto scenario_model = ScenarioGenerator::generate(
        ScenarioGeneratorOptions{
            WorldGenerationOptions(),
            vector<TaskGenerationOptions>(1, TaskGenerationOptions())
        }
    );

    cout << scenario_model.world().id() << endl;

    return 0;
}
