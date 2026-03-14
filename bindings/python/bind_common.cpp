/**
 * @file bind_common.cpp
 * @brief Binds common-domain enums, value types, and helpers to Python.
 */
#include "bind.hpp"

#include <sstream>
#include <pybind11/stl.h>

#include <skymapf/common/coord.hpp>
#include <skymapf/common/index.hpp>
#include <skymapf/common/move.hpp>
#include <skymapf/common/space.hpp>
#include <skymapf/common/time.hpp>
#include <skymapf/common/units.hpp>

namespace py = pybind11;

namespace skymapf::python {

void bind_common(py::module_& m) {
    auto common_m = m.def_submodule("common", "Common domain objects and helpers.");
    using GridShape2DCols = decltype(common::GridShape2D::cols);
    using GridShape2DRows = decltype(common::GridShape2D::rows);
    using GridShape3DCols = decltype(common::GridShape3D::cols);
    using GridShape3DRows = decltype(common::GridShape3D::rows);
    using GridShape3DLayers = decltype(common::GridShape3D::layers);
    using CellCoord2DX = decltype(common::CellCoord2D::x);
    using CellCoord2DY = decltype(common::CellCoord2D::y);
    using CellCoord3DX = decltype(common::CellCoord3D::x);
    using CellCoord3DY = decltype(common::CellCoord3D::y);
    using CellCoord3DZ = decltype(common::CellCoord3D::z);

    auto space_kind_name = [](common::SpaceKind kind) -> const char* {
        switch (kind) {
            case common::SpaceKind::Plane2D:
                return "Plane2D";
            case common::SpaceKind::Space3D:
                return "Space3D";
            default:
                return "Unknown";
        }
    };

    auto length_unit_name = [](common::LengthUnit unit) -> const char* {
        switch (unit) {
            case common::LengthUnit::Meter:
                return "Meter";
            case common::LengthUnit::Centimeter:
                return "Centimeter";
            case common::LengthUnit::Millimeter:
                return "Millimeter";
            default:
                return "Unknown";
        }
    };

    py::enum_<common::SpaceKind>(common_m, "SpaceKind")
        .value("Plane2D", common::SpaceKind::Plane2D)
        .value("Space3D", common::SpaceKind::Space3D);

    py::class_<common::GridShape2D>(common_m, "GridShape2D")
        .def(py::init<>())
        .def(py::init<GridShape2DCols, GridShape2DRows>(), py::arg("cols"), py::arg("rows"))
        .def_readwrite("cols", &common::GridShape2D::cols)
        .def_readwrite("rows", &common::GridShape2D::rows)
        .def("__repr__", [](const common::GridShape2D& self) {
            std::ostringstream oss;
            oss << "GridShape2D(cols=" << self.cols << ", rows=" << self.rows << ")";
            return oss.str();
        });

    py::class_<common::GridShape3D>(common_m, "GridShape3D")
        .def(py::init<>())
        .def(
            py::init<GridShape3DCols, GridShape3DRows, GridShape3DLayers>(),
            py::arg("cols"),
            py::arg("rows"),
            py::arg("layers")
        )
        .def_readwrite("cols", &common::GridShape3D::cols)
        .def_readwrite("rows", &common::GridShape3D::rows)
        .def_readwrite("layers", &common::GridShape3D::layers)
        .def("__repr__", [](const common::GridShape3D& self) {
            std::ostringstream oss;
            oss << "GridShape3D(cols=" << self.cols << ", rows=" << self.rows
                << ", layers=" << self.layers << ")";
            return oss.str();
        });

    py::class_<common::SpaceSpec>(common_m, "SpaceSpec")
        .def(py::init<>())
        .def_static(
            "make_2d",
            &common::SpaceSpec::make_2d,
            py::arg("cols"),
            py::arg("rows")
        )
        .def_static(
            "make_3d",
            &common::SpaceSpec::make_3d,
            py::arg("cols"),
            py::arg("rows"),
            py::arg("layers")
        )
        .def("kind", &common::SpaceSpec::kind)
        .def("is_2d", &common::SpaceSpec::is_2d)
        .def("is_3d", &common::SpaceSpec::is_3d)
        .def("cols", &common::SpaceSpec::cols)
        .def("rows", &common::SpaceSpec::rows)
        .def("layers", &common::SpaceSpec::layers)
        // shape_2d()/shape_3d() return value objects in core (not references),
        // so the default return value policy is already appropriate.
        .def("shape_2d", &common::SpaceSpec::shape_2d)
        .def("shape_3d", &common::SpaceSpec::shape_3d)
        .def("is_valid", &common::SpaceSpec::is_valid)
        .def("__repr__", [space_kind_name](const common::SpaceSpec& self) {
            std::ostringstream oss;
            oss << "SpaceSpec(kind=" << space_kind_name(self.kind()) << ", cols=" << self.cols()
                << ", rows=" << self.rows();
            if (self.is_3d()) {
                oss << ", layers=" << self.layers();
            }
            oss << ")";
            return oss.str();
        });

    py::class_<common::CellCoord2D>(common_m, "CellCoord2D")
        .def(py::init<>())
        .def(py::init<CellCoord2DX, CellCoord2DY>(), py::arg("x"), py::arg("y"))
        .def_readwrite("x", &common::CellCoord2D::x)
        .def_readwrite("y", &common::CellCoord2D::y)
        .def("__repr__", [](const common::CellCoord2D& self) {
            std::ostringstream oss;
            oss << "CellCoord2D(x=" << self.x << ", y=" << self.y << ")";
            return oss.str();
        });

    py::class_<common::CellCoord3D>(common_m, "CellCoord3D")
        .def(py::init<>())
        .def(py::init<CellCoord3DX, CellCoord3DY, CellCoord3DZ>(), py::arg("x"), py::arg("y"), py::arg("z"))
        .def_readwrite("x", &common::CellCoord3D::x)
        .def_readwrite("y", &common::CellCoord3D::y)
        .def_readwrite("z", &common::CellCoord3D::z)
        .def("__repr__", [](const common::CellCoord3D& self) {
            std::ostringstream oss;
            oss << "CellCoord3D(x=" << self.x << ", y=" << self.y << ", z=" << self.z << ")";
            return oss.str();
        });

    py::enum_<common::MoveDir4>(common_m, "MoveDir4")
        .value("Up", common::MoveDir4::Up)
        .value("Down", common::MoveDir4::Down)
        .value("Left", common::MoveDir4::Left)
        .value("Right", common::MoveDir4::Right);

    py::enum_<common::MoveDir6>(common_m, "MoveDir6")
        .value("PosX", common::MoveDir6::PosX)
        .value("NegX", common::MoveDir6::NegX)
        .value("PosY", common::MoveDir6::PosY)
        .value("NegY", common::MoveDir6::NegY)
        .value("PosZ", common::MoveDir6::PosZ)
        .value("NegZ", common::MoveDir6::NegZ);

    py::enum_<common::LengthUnit>(common_m, "LengthUnit")
        .value("Meter", common::LengthUnit::Meter)
        .value("Centimeter", common::LengthUnit::Centimeter)
        .value("Millimeter", common::LengthUnit::Millimeter);

    py::class_<common::GridUnitSpec>(common_m, "GridUnitSpec")
        .def(py::init<>())
        .def(py::init<double, common::LengthUnit>(), py::arg("cell_size"), py::arg("length_unit"))
        .def_readwrite("cell_size", &common::GridUnitSpec::cell_size)
        .def_readwrite("length_unit", &common::GridUnitSpec::length_unit)
        .def("__repr__", [length_unit_name](const common::GridUnitSpec& self) {
            std::ostringstream oss;
            oss << "GridUnitSpec(cell_size=" << self.cell_size
                << ", length_unit=" << length_unit_name(self.length_unit) << ")";
            return oss.str();
        });

    common_m.attr("kInvalidTimeStep") = py::int_(common::kInvalidTimeStep);

    common_m.def(
        "is_valid_shape_2d",
        py::overload_cast<common::GridShape2D>(&common::is_valid),
        py::arg("shape")
    );
    common_m.def(
        "is_valid_shape_3d",
        py::overload_cast<common::GridShape3D>(&common::is_valid),
        py::arg("shape")
    );
    common_m.def(
        "is_valid_unit_spec",
        py::overload_cast<common::GridUnitSpec>(&common::is_valid),
        py::arg("unit_spec")
    );
    // Behavior for invalid sentinel values (e.g. kInvalidTimeStep) follows core semantics.
    common_m.def("next_time", &common::next_time, py::arg("time_step"));

    common_m.def(
        "is_in_bounds_2d",
        py::overload_cast<const common::CellCoord2D&, common::GridShape2D>(&common::is_in_bounds),
        py::arg("coord"),
        py::arg("shape")
    );
    common_m.def(
        "is_in_bounds_3d",
        py::overload_cast<const common::CellCoord3D&, common::GridShape3D>(&common::is_in_bounds),
        py::arg("coord"),
        py::arg("shape")
    );
    common_m.def(
        "to_index_2d",
        py::overload_cast<const common::CellCoord2D&, common::GridShape2D>(&common::to_index),
        py::arg("coord"),
        py::arg("shape")
    );
    common_m.def(
        "to_index_3d",
        py::overload_cast<const common::CellCoord3D&, common::GridShape3D>(&common::to_index),
        py::arg("coord"),
        py::arg("shape")
    );
    common_m.def(
        "to_coord_2d",
        &common::to_coord_2d,
        py::arg("index"),
        py::arg("shape")
    );
    common_m.def(
        "to_coord_3d",
        &common::to_coord_3d,
        py::arg("index"),
        py::arg("shape")
    );
}

}  // namespace skymapf::python
