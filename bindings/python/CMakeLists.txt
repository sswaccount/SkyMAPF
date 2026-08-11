if(NOT TARGET pybind11::module)
  message(FATAL_ERROR "pybind11 is required for SKYMAPF_BUILD_PYTHON_BINDINGS=ON")
endif()

pybind11_add_module(skymapf_core_py
    module.cpp
    bind_version.cpp
    bind_common.cpp
)

target_link_libraries(skymapf_core_py
    PRIVATE
        SkyMAPF::core
)

set_target_properties(skymapf_core_py PROPERTIES
    OUTPUT_NAME "_core"
)
include(GNUInstallDirs)
if(DEFINED SKBUILD_PLATLIB_DIR)
    install(TARGETS skymapf_core_py
        LIBRARY DESTINATION ${SKBUILD_PLATLIB_DIR}/skymapf
        RUNTIME DESTINATION ${SKBUILD_PLATLIB_DIR}/skymapf
    )
else()
    install(TARGETS skymapf_core_py
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/python/skymapf
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/python/skymapf
    )
endif()
