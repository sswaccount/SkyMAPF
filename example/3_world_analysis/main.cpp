/**
 * @file main.cpp
 * @brief Reads an ASCII map and reports obstacle density and component count.
 *
 * Map format:
 * - '.' walkable
 * - '#' obstacle
 * - all non-empty lines must have equal width
 */
#include <fstream>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <skymapf/common/index.hpp>
#include <skymapf/common/space.hpp>
#include <skymapf/world/analysis.hpp>
#include <skymapf/world/model.hpp>

namespace {

bool build_world_from_ascii(
    const std::vector<std::string>& rows,
    skymapf::world::WorldModel* world_out,
    std::string* error
) {
    if (rows.empty()) {
        if (error) {
            *error = "Map is empty.";
        }
        return false;
    }

    const auto row_count = static_cast<std::int32_t>(rows.size());
    const auto col_count = static_cast<std::int32_t>(rows.front().size());
    if (col_count <= 0) {
        if (error) {
            *error = "Map has empty first row.";
        }
        return false;
    }
    for (const auto& row : rows) {
        if (static_cast<std::int32_t>(row.size()) != col_count) {
            if (error) {
                *error = "Map rows have inconsistent widths.";
            }
            return false;
        }
    }

    skymapf::world::WorldModel world(
        static_cast<skymapf::common::WorldId>(1),
        std::string{},
        skymapf::common::SpaceSpec::make_2d(col_count, row_count)
    );
    for (std::int32_t y = 0; y < row_count; ++y) {
        for (std::int32_t x = 0; x < col_count; ++x) {
            const auto ch = rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            if (ch != '.' && ch != '#') {
                if (error) {
                    *error = "Map contains invalid char; only '.' and '#' are allowed.";
                }
                return false;
            }
            const auto idx = skymapf::common::to_index(
                skymapf::common::CellCoord2D{x, y},
                world.space_spec().shape_2d()
            );
            world.set_walkable(idx, ch == '.');
        }
    }

    if (world_out) {
        *world_out = std::move(world);
    }
    return true;
}

bool load_ascii_map_file(const std::string& path, std::vector<std::string>* rows, std::string* error) {
    std::ifstream in(path);
    if (!in) {
        if (error) {
            *error = "Cannot open file: " + path;
        }
        return false;
    }

    std::vector<std::string> loaded;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty()) {
            loaded.push_back(line);
        }
    }

    if (rows) {
        *rows = std::move(loaded);
    }
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: world_analysis <ascii_map_file>\n";
        std::cerr << "Map chars: '.' walkable, '#' obstacle.\n";
        return 1;
    }

    std::vector<std::string> rows;
    std::string error;
    if (!load_ascii_map_file(argv[1], &rows, &error)) {
        std::cerr << "Error: " << error << '\n';
        return 1;
    }

    skymapf::world::WorldModel world(
        static_cast<skymapf::common::WorldId>(1),
        std::string{},
        skymapf::common::SpaceSpec::make_2d(1, 1)
    );
    if (!build_world_from_ascii(rows, &world, &error)) {
        std::cerr << "Error: " << error << '\n';
        return 1;
    }

    const auto stats = skymapf::world::analyze_world(
        world,
        skymapf::world::ComponentNeighborhoodMode::Geometric
    );

    std::cout << "rows=" << rows.size()
              << ", cols=" << rows.front().size()
              << ", total_cells=" << stats.total_cells << '\n';
    std::cout << "obstacle_cells=" << stats.obstacle_cells
              << ", walkable_cells=" << stats.walkable_cells << '\n';
    std::cout << "obstacle_density=" << stats.obstacle_density << '\n';
    std::cout << "walkable_components=" << stats.walkable_component_count << '\n';
    return 0;
}
