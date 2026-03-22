/**
 * @file space.hpp
 * @brief Defines discrete space kinds, shapes, and validation helpers.
 */
#pragma once

#include <cstdint>

#include "ids.hpp"

namespace skymapf::common {

/// Classifies whether a world uses 2D or 3D discrete space.
enum class SpaceKind {
    Plane2D,
    Space3D
};

/// Stores 2D grid shape in (cols, rows) order.
struct GridShape2D {
    std::int32_t cols{0};
    std::int32_t rows{0};
};

/// Stores 3D grid shape in (cols, rows, layers) order.
struct GridShape3D {
    std::int32_t cols{0};
    std::int32_t rows{0};
    std::int32_t layers{0};
};

/**
 * @brief Represents the discrete dimensions of a world.
 *
 * The type is mutable and supports both 2D and 3D configurations.
 * A 2D space always uses exactly one layer.
 */
class SpaceSpec {
public:
    SpaceSpec() = default;

    static SpaceSpec make_2d(std::int32_t cols, std::int32_t rows) {
        SpaceSpec spec;
        spec.kind_ = SpaceKind::Plane2D;
        spec.cols_ = cols;
        spec.rows_ = rows;
        spec.layers_ = 1;
        return spec;
    }

    static SpaceSpec make_3d(std::int32_t cols, std::int32_t rows, std::int32_t layers) {
        SpaceSpec spec;
        spec.kind_ = SpaceKind::Space3D;
        spec.cols_ = cols;
        spec.rows_ = rows;
        spec.layers_ = layers;
        return spec;
    }

    SpaceKind kind() const noexcept { return kind_; }
    bool is_2d() const noexcept { return kind_ == SpaceKind::Plane2D; }
    bool is_3d() const noexcept { return kind_ == SpaceKind::Space3D; }

    std::int32_t cols() const noexcept { return cols_; }
    std::int32_t rows() const noexcept { return rows_; }
    std::int32_t layers() const noexcept { return layers_; }

    GridShape2D shape_2d() const noexcept { return GridShape2D{cols_, rows_}; }
    GridShape3D shape_3d() const noexcept { return GridShape3D{cols_, rows_, layers_}; }

    bool is_valid() const noexcept {
        if (cols_ <= 0 || rows_ <= 0) {
            return false;
        }
        if (is_2d()) {
            return layers_ == 1;
        }
        return layers_ > 0;
    }

private:
    SpaceKind kind_{SpaceKind::Plane2D};
    std::int32_t cols_{0};
    std::int32_t rows_{0};
    std::int32_t layers_{1};
};

inline bool is_valid(GridShape2D shape) noexcept {
    return shape.cols > 0 && shape.rows > 0;
}

inline bool is_valid(GridShape3D shape) noexcept {
    return shape.cols > 0 && shape.rows > 0 && shape.layers > 0;
}

/**
 * @brief Returns total cell count represented by a space specification.
 *
 * Invalid space specifications return zero.
 */
inline CellIndex cell_count(const SpaceSpec& spec) noexcept {
    if (!spec.is_valid()) {
        return 0;
    }
    if (spec.is_2d()) {
        const auto shape = spec.shape_2d();
        return static_cast<CellIndex>(shape.cols) * static_cast<CellIndex>(shape.rows);
    }
    const auto shape = spec.shape_3d();
    return static_cast<CellIndex>(shape.cols)
         * static_cast<CellIndex>(shape.rows)
         * static_cast<CellIndex>(shape.layers);
}

}  // namespace skymapf::common