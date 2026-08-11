/**
 * @file module.cpp
 * @brief Defines the Python extension module entrypoint.
 */
 #include <pybind11/pybind11.h>

 #include "bind.hpp"
 
 namespace py = pybind11;
 
 PYBIND11_MODULE(_core, m) {
     m.doc() = "Optional native extension for the SkyMAPF Python SDK";
 
     skymapf::python::bind_version(m);
     skymapf::python::bind_common(m);
 }
