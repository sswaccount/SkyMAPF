#pragma once

#include <pybind11/pybind11.h>

namespace skymapf::python {

void bind_version(pybind11::module_& m);
void bind_common(pybind11::module_& m);

}  // namespace skymapf::python