#include "bind.hpp"

#include <skymapf/version.hpp>

namespace skymapf::python {

void bind_version(pybind11::module_& m) {
    m.attr("__version__") = skymapf::kVersionString;
}

}  // namespace skymapf::python