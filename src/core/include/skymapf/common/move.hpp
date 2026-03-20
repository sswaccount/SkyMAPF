#pragma once

#include <array>

#include "coord.hpp"

namespace skymapf::common {

enum class MoveDir4 {
    Up,
    Down,
    Left,
    Right,
};

enum class MoveDir6 {
    PosX,
    NegX,
    PosY,
    NegY,
    PosZ,
    NegZ,
};

inline constexpr std::array<CellCoord2D, 4> kMoveDelta4{
    CellCoord2D{0, -1},
    CellCoord2D{0, 1},
    CellCoord2D{-1, 0},
    CellCoord2D{1, 0},
};

inline constexpr std::array<CellCoord3D, 6> kMoveDelta6{
    CellCoord3D{1, 0, 0},
    CellCoord3D{-1, 0, 0},
    CellCoord3D{0, 1, 0},
    CellCoord3D{0, -1, 0},
    CellCoord3D{0, 0, 1},
    CellCoord3D{0, 0, -1},
};

}  // namespace skymapf::common
