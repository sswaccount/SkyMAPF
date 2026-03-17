#pragma once

namespace skymapf::common {

enum class SpaceKind {
    Plane2D,
    Space3D
};

class SpaceSpec {
public:
    SpaceSpec() = default;

    static SpaceSpec make_2d(double width, double height) {
        SpaceSpec spec;
        spec.kind_ = SpaceKind::Plane2D;
        spec.width_ = width;
        spec.height_ = height;
        spec.depth_ = 0.0;
        return spec;
    }
    static SpaceSpec make_3d(double width, double height, double depth) {
        SpaceSpec spec;
        spec.kind_ = SpaceKind::Space3D;
        spec.width_ = width;
        spec.height_ = height;
        spec.depth_ = depth;
        return spec;
    }

    SpaceKind kind() const noexcept { return kind_; }

    double width() const noexcept { return width_; }
    double height() const noexcept { return height_; }
    double depth() const noexcept { return depth_; }

    bool is_2d() const noexcept { return kind_ == SpaceKind::Plane2D; }
    bool is_3d() const noexcept { return kind_ == SpaceKind::Space3D; }

private:
    SpaceKind kind_{SpaceKind::Plane2D};
    double width_{0.0};
    double height_{0.0};
    double depth_{0.0};
};

}