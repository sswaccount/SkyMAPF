/**
 * @file move.hpp
 * @brief Defines canonical neighbor movement directions for grids.
 */
#pragma once

#include <array>

#include "coord.hpp"

namespace skymapf::common {

/// Enumerates cardinal moves in a 2D grid.
enum class MoveDir4 {
    Up,
    Down,
    Left,
    Right,
};

/// Enumerates axis-aligned moves in a 3D grid.
enum class MoveDir6 {
    PosX,
    NegX,
    PosY,
    NegY,
    PosZ,
    NegZ,
};

/// Delta table for 2D cardinal moves in (x, y).
inline constexpr std::array<CellCoord2D, 4> kMoveDelta4{
    CellCoord2D{0, -1},
    CellCoord2D{0, 1},
    CellCoord2D{-1, 0},
    CellCoord2D{1, 0},
};

/// Delta table for 3D axis-aligned moves in (x, y, z).
inline constexpr std::array<CellCoord3D, 6> kMoveDelta6{
    CellCoord3D{1, 0, 0},
    CellCoord3D{-1, 0, 0},
    CellCoord3D{0, 1, 0},
    CellCoord3D{0, -1, 0},
    CellCoord3D{0, 0, 1},
    CellCoord3D{0, 0, -1},
};

}  // namespace skymapf::common
