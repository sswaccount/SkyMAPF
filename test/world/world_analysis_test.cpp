/**
 * @file world_analysis_test.cpp
 * @brief Verifies obstacle density and component counting on small 2D maps.
 */
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include <skymapf/common/index.hpp>
#include <skymapf/common/space.hpp>
#include <skymapf/world/analysis.hpp>
#include <skymapf/world/model.hpp>

namespace {

skymapf::world::WorldModel build_world_from_ascii(const std::vector<std::string>& rows) {
    REQUIRE(!rows.empty());
    const auto row_count = static_cast<std::int32_t>(rows.size());
    const auto col_count = static_cast<std::int32_t>(rows.front().size());
    REQUIRE(col_count > 0);
    for (const auto& row : rows) {
        REQUIRE(static_cast<std::int32_t>(row.size()) == col_count);
    }

    skymapf::world::WorldModel world(
        static_cast<skymapf::common::WorldId>(1),
        std::string{},
        skymapf::common::SpaceSpec::make_2d(col_count, row_count)
    );

    for (std::int32_t y = 0; y < row_count; ++y) {
        for (std::int32_t x = 0; x < col_count; ++x) {
            const auto ch = rows[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            const bool walkable = (ch == '.');
            const auto idx = skymapf::common::to_index(
                skymapf::common::CellCoord2D{x, y},
                world.space_spec().shape_2d()
            );
            world.set_walkable(idx, walkable);
        }
    }
    return world;
}

void check_case(
    const std::vector<std::string>& map_rows,
    double expected_density,
    std::size_t expected_components
) {
    const auto world = build_world_from_ascii(map_rows);
    const auto result = skymapf::world::analyze_world(
        world,
        skymapf::world::ComponentNeighborhoodMode::Geometric
    );
    CHECK(std::abs(result.obstacle_density - expected_density) < 1e-12);
    CHECK(result.walkable_component_count == expected_components);
}

}  // namespace

TEST_CASE("Case1 6x6 no obstacles", "[world][analysis]") {
    check_case(
        {
            "......",
            "......",
            "......",
            "......",
            "......",
            "......",
        },
        0.0,
        1
    );
}

TEST_CASE("Case2 6x6 two walkable components split by obstacle wall", "[world][analysis]") {
    check_case(
        {
            "......",
            "..##..",
            "######",
            "......",
            ".####.",
            "......",
        },
        12.0 / 36.0,
        2
    );
}

TEST_CASE("Case3 8x8 cross walls yielding four components", "[world][analysis]") {
    check_case(
        {
            "...#....",
            "...#....",
            "...#....",
            "########",
            "...#....",
            "...#....",
            "...#....",
            "...#....",
        },
        15.0 / 64.0,
        4
    );
}

TEST_CASE("Case4 6x6 sparse walkable islands (three components)", "[world][analysis]") {
    check_case(
        {
            "#.####",
            "######",
            "###.##",
            "######",
            "####.#",
            "######",
        },
        33.0 / 36.0,
        3
    );
}

TEST_CASE("Case5 8x8 random-like connected corridor map", "[world][analysis]") {
    check_case(
        {
            "##....##",
            "##.##.##",
            "##.##.##",
            "##....##",
            "####.###",
            "###..###",
            "##....##",
            "########",
        },
        45.0 / 64.0,
        1
    );
}
