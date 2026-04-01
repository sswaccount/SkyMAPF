#include "skymapf/io/scenario_io.hpp"
#include "skymapf/scenario/model.hpp"

#include <filesystem>
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    try {
        const auto root = (argc >= 2)
            ? std::filesystem::path(argv[1])
            : (std::filesystem::current_path() / "data");

        if (argc < 3) {
            std::cerr
                << "Usage: " << (argc > 0 ? argv[0] : "example3")
                << " <root_dir> <scenario_folder_name>\n"
                << "Example: " << (argc > 0 ? argv[0] : "example3")
                << " data scenario_abc123\n";
            return 1;
        }
        const std::string scenario_name = argv[2];

        const skymapf::io::ScenarioIO io(root);
        const skymapf::scenario::ScenarioModel scenario = io.read(scenario_name);
        const auto meta = scenario.meta();

        std::cout << "[OK] Scenario loaded and validated.\n";
        std::cout << "Root: " << root.string() << "\n";
        std::cout << "Folder: " << scenario_name << "\n";
        std::cout << "Scenario id: " << scenario.id() << "\n";
        std::cout << "Scenario name: " << scenario.name() << "\n";
        std::cout << "Scenario info: " << scenario.info() << "\n";
        std::cout << "Random seed: " << scenario.random_seed() << "\n";
        std::cout << "World id: " << scenario.world().id() << "\n";
        std::cout << "Task count: " << scenario.tasks().size() << "\n";
        std::cout << "Meta checksum: " << meta.checksum << "\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] Scenario read/validation failed: " << ex.what() << "\n";
        return 2;
    }
}

