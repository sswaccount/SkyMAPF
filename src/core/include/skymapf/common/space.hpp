#pragma once

#include <cstdint>

namespace skymapf::common {

enum class SpaceKind {
    Plane2D,
    Space3D
};

struct GridShape2D {
    std::int32_t cols{0};
    std::int32_t rows{0};
};

struct GridShape3D {
    std::int32_t cols{0};
    std::int32_t rows{0};
    std::int32_t layers{0};
};

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

}