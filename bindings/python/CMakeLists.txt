if(NOT TARGET pybind11::module)
  message(FATAL_ERROR "pybind11 is required for SKYMAPF_BUILD_PYTHON_BINDINGS=ON")
endif()

pybind11_add_module(skymapf_py
    module.cpp
    bind_version.cpp
    bind_common.cpp
)

target_link_libraries(skymapf_py
    PRIVATE
        SkyMAPF::core
)

set_target_properties(skymapf_py PROPERTIES
    OUTPUT_NAME "skymapf"
)

install(TARGETS skymapf_py
    LIBRARY DESTINATION ${SKBUILD_PLATLIB_DIR}
    RUNTIME DESTINATION ${SKBUILD_PLATLIB_DIR}
)