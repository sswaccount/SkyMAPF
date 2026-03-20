# ---------------------------------------------------------------------------
# src/cli/CMakeLists.txt
# Build CLI executable as a separate entrypoint target.
# ---------------------------------------------------------------------------

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    message(STATUS "src/cli/main.cpp not found. Skipping skymapf target.")
    return()
endif()

add_executable(skymapf
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/build_app.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/commands/register_version.cpp"
)

target_link_libraries(skymapf
    PRIVATE
        SkyMAPF::core
        CLI11::CLI11
)

target_include_directories(skymapf
    PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
)

target_compile_features(skymapf PRIVATE cxx_std_17)

set_target_properties(skymapf PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

skymapf_apply_warnings(skymapf)