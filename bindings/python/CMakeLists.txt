if(NOT TARGET pybind11::module)
  message(FATAL_ERROR "pybind11 is required for SKYMAPF_BUILD_PYTHON_BINDINGS=ON")
endif()

pybind11_add_module(skymapf MODULE module.cpp)
target_link_libraries(skymapf PRIVATE SkyMAPF::core)