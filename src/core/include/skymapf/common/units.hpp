/**
 * @file units.hpp
 * @brief Defines optional real-world unit metadata for grid cells.
 */
#pragma once

namespace skymapf::common {

/// Physical length units for map metadata.
enum class LengthUnit {
    Meter,
    Centimeter,
    Millimeter,
};

/// Describes how one grid cell maps to physical length.
struct GridUnitSpec {
    double cell_size{1.0};
    LengthUnit length_unit{LengthUnit::Meter};
};

/// Returns whether unit metadata is internally valid.
inline bool is_valid(GridUnitSpec spec) noexcept {
    return spec.cell_size > 0.0;
}

}  // namespace skymapf::common
