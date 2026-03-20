/**
 * @file module.cpp
 * @brief Defines the Python extension module entrypoint.
 */
 #include <pybind11/pybind11.h>

 #include "bind.hpp"
 
 namespace py = pybind11;
 
 PYBIND11_MODULE(skymapf, m) {
     m.doc() = "SkyMAPF Python bindings";
 
     skymapf::python::bind_version(m);
     skymapf::python::bind_common(m);
 }