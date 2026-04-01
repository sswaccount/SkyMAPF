#include "skymapf/common/space.hpp"
#include "skymapf/generator/task_generator.hpp"
#include "skymapf/generator/world_generator.hpp"
#include "skymapf/io/scenario_io.hpp"
#include "skymapf/scenario/model.hpp"
#include "skymapf/utils/random_tool.hpp"
#include <nlohmann/json.hpp>

#include <filesystem>
#include <exception>
#include <iostream>
#include <vector>

int main() {
    try {
        skymapf::utils::RandomTool::instance().configure_deterministic(42);

        auto world_model = skymapf::generator::WorldGenerator::generate(
            skymapf::generator::WorldGenerationOptions(
                skymapf::common::SpaceSpec::make_2d(8, 8)
            )
        );
        auto task_models = skymapf::generator::TaskGenerator::generate(
            world_model,
            {
                skymapf::generator::TaskGenerationOptions(4),
                skymapf::generator::TaskGenerationOptions(5)
            }
        );

        // Pass an empty scenario name so the model auto-generates a random name.
        auto scenario_model = skymapf::scenario::ScenarioModel(
            std::move(world_model),
            std::move(task_models)
        );
        scenario_model.set_info("example2 write-to-data demonstration");

        const auto data_root = std::filesystem::current_path() / "data";
        const skymapf::io::ScenarioIO io(data_root);
        const auto written_directory = io.write(scenario_model);

        std::cout << "Scenario name: " << scenario_model.name() << '\n';
        std::cout << "Written directory: " << written_directory.string() << '\n';

        // Optional: verify one full read path.
        const auto loaded = io.read(written_directory.filename().string());
        nlohmann::ordered_json loaded_meta = loaded.meta();
        std::cout << "Loaded meta: " << loaded_meta.dump(2) << '\n';
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "example2 failed: " << ex.what() << '\n';
        return 1;
    }
}
