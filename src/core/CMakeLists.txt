# ---------------------------------------------------------------------------
# src/core/CMakeLists.txt
# Build the reusable core library (no main function here).
# ---------------------------------------------------------------------------

file(GLOB_RECURSE SKYMAPF_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
)

if(NOT SKYMAPF_CORE_SOURCES)
    message(FATAL_ERROR
        "No core sources found under src/core/src. "
        "Please add at least one *.cpp file for skymapf_core."
    )
endif()

add_library(skymapf_core ${SKYMAPF_CORE_SOURCES})
add_library(SkyMAPF::core ALIAS skymapf_core)

target_compile_features(skymapf_core PUBLIC cxx_std_17)
target_include_directories(skymapf_core
    PUBLIC
        "$<BUILD_INTERFACE:${SKYMAPF_GENERATED_INCLUDE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(skymapf_core PUBLIC nlohmann_json::nlohmann_json)

skymapf_apply_warnings(skymapf_core)

set_target_properties(skymapf_core PROPERTIES
    EXPORT_NAME core
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
)

if(SKYMAPF_INSTALL_CPP_SDK)
    install(TARGETS skymapf_core
        EXPORT SkyMAPFTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/skymapf"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        FILES_MATCHING PATTERN "*.hpp"
    )
    install(FILES "${SKYMAPF_GENERATED_INCLUDE_DIR}/skymapf/version.hpp"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/skymapf"
    )
endif()
