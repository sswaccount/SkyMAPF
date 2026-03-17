#include <pybind11/pybind11.h>
#include <skymapf/version.hpp>

PYBIND11_MODULE(skymapf, m) {
    m.doc() = "SkyMAPF Python bindings";

    m.attr("__version__") = skymapf::kVersionString;
}