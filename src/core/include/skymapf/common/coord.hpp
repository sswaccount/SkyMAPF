/**
 * @file coord.hpp
 * @brief Defines integer grid coordinate types for 2D and 3D spaces.
 */
#pragma once

#include <cstdint>

namespace skymapf::common {

/// Represents a 2D cell coordinate in (x, y) order.
struct CellCoord2D {
    std::int32_t x{0};
    std::int32_t y{0};

    bool operator==(const CellCoord2D& rhs) const noexcept {
        return x == rhs.x && y == rhs.y;
    }
};

/// Represents a 3D cell coordinate in (x, y, z) order.
struct CellCoord3D {
    std::int32_t x{0};
    std::int32_t y{0};
    std::int32_t z{0};

    bool operator==(const CellCoord3D& rhs) const noexcept {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }
};

}  // namespace skymapf::common
