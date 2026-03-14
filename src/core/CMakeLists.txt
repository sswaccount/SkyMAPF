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
)
target_link_libraries(skymapf_core PUBLIC nlohmann_json::nlohmann_json)

skymapf_apply_warnings(skymapf_core)
