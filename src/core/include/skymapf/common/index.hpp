/**
 * @file index.hpp
 * @brief Provides coordinate/index conversion and bounds checks.
 */
#pragma once

#include <cstdint>

#include "coord.hpp"
#include "ids.hpp"
#include "space.hpp"

namespace skymapf::common {

/// Returns whether a 2D coordinate lies within a shape.
inline bool is_in_bounds(const CellCoord2D& coord, GridShape2D shape) noexcept {
    return coord.x >= 0 && coord.y >= 0 && coord.x < shape.cols && coord.y < shape.rows;
}

/// Returns whether a 3D coordinate lies within a shape.
inline bool is_in_bounds(const CellCoord3D& coord, GridShape3D shape) noexcept {
    return coord.x >= 0 && coord.y >= 0 && coord.z >= 0
        && coord.x < shape.cols && coord.y < shape.rows && coord.z < shape.layers;
}

/// Converts a 2D coordinate to row-major linear index.
inline CellIndex to_index(const CellCoord2D& coord, GridShape2D shape) noexcept {
    return static_cast<CellIndex>(coord.y) * static_cast<CellIndex>(shape.cols)
         + static_cast<CellIndex>(coord.x);
}

/// Converts a 3D coordinate to layer-major linear index.
inline CellIndex to_index(const CellCoord3D& coord, GridShape3D shape) noexcept {
    return static_cast<CellIndex>(coord.z) * static_cast<CellIndex>(shape.rows) * static_cast<CellIndex>(shape.cols)
         + static_cast<CellIndex>(coord.y) * static_cast<CellIndex>(shape.cols)
         + static_cast<CellIndex>(coord.x);
}

/// Converts a row-major index to a 2D coordinate.
inline CellCoord2D to_coord_2d(CellIndex index, GridShape2D shape) noexcept {
    const auto cols = static_cast<CellIndex>(shape.cols);
    return CellCoord2D{
        static_cast<std::int32_t>(index % cols),
        static_cast<std::int32_t>(index / cols),
    };
}

/// Converts a layer-major index to a 3D coordinate.
inline CellCoord3D to_coord_3d(CellIndex index, GridShape3D shape) noexcept {
    const auto cols = static_cast<CellIndex>(shape.cols);
    const auto rows = static_cast<CellIndex>(shape.rows);
    const auto layer_size = cols * rows;
    const auto z = index / layer_size;
    const auto rem = index % layer_size;
    const auto y = rem / cols;
    const auto x = rem % cols;
    return CellCoord3D{
        static_cast<std::int32_t>(x),
        static_cast<std::int32_t>(y),
        static_cast<std::int32_t>(z),
    };
}

}  // namespace skymapf::common
