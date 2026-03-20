#pragma once

namespace skymapf::common {

enum class LengthUnit {
    Meter,
    Centimeter,
    Millimeter,
};

struct GridUnitSpec {
    double cell_size{1.0};
    LengthUnit length_unit{LengthUnit::Meter};
};

inline bool is_valid(GridUnitSpec spec) noexcept {
    return spec.cell_size > 0.0;
}

}  // namespace skymapf::common
