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
    /// Constructs an empty (invalid) default space spec.
    SpaceSpec() = default;

    /**
     * @brief Creates a valid 2D space specification.
     *
     * The resulting spec uses exactly one layer.
     *
     * @param cols Number of columns.
     * @param rows Number of rows.
     * @return Initialized 2D space specification.
     */
    static SpaceSpec make_2d(std::int32_t cols, std::int32_t rows) {
        SpaceSpec spec;
        spec.kind_ = SpaceKind::Plane2D;
        spec.cols_ = cols;
        spec.rows_ = rows;
        spec.layers_ = 1;
        return spec;
    }

    /**
     * @brief Creates a valid 3D space specification.
     *
     * @param cols Number of columns.
     * @param rows Number of rows.
     * @param layers Number of layers.
     * @return Initialized 3D space specification.
     */
    static SpaceSpec make_3d(std::int32_t cols, std::int32_t rows, std::int32_t layers) {
        SpaceSpec spec;
        spec.kind_ = SpaceKind::Space3D;
        spec.cols_ = cols;
        spec.rows_ = rows;
        spec.layers_ = layers;
        return spec;
    }

    /// Returns whether this spec is 2D or 3D.
    SpaceKind kind() const noexcept { return kind_; }
    /// Returns true when this spec describes a 2D space.
    bool is_2d() const noexcept { return kind_ == SpaceKind::Plane2D; }
    /// Returns true when this spec describes a 3D space.
    bool is_3d() const noexcept { return kind_ == SpaceKind::Space3D; }

    /// Returns column size.
    std::int32_t cols() const noexcept { return cols_; }
    /// Returns row size.
    std::int32_t rows() const noexcept { return rows_; }
    /// Returns layer size (always 1 for valid 2D specs).
    std::int32_t layers() const noexcept { return layers_; }

    /// Returns shape view in 2D layout (cols, rows).
    GridShape2D shape_2d() const noexcept { return GridShape2D{cols_, rows_}; }
    /// Returns shape view in 3D layout (cols, rows, layers).
    GridShape3D shape_3d() const noexcept { return GridShape3D{cols_, rows_, layers_}; }

    /**
     * @brief Validates shape constraints against current space kind.
     *
     * Rules:
     * - cols and rows must be positive.
     * - 2D specs must have exactly one layer.
     * - 3D specs must have positive layers.
     *
     * @return True when the shape is internally consistent and usable.
     */
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