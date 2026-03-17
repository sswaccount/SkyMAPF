# ---------------------------------------------------------------------------
# src/cli/CMakeLists.txt
# Build CLI executable as a separate entrypoint target.
# ---------------------------------------------------------------------------

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    message(STATUS "src/cli/main.cpp not found. Skipping skymapf_cli target.")
    return()
endif()

add_executable(skymapf_cli "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
target_link_libraries(skymapf_cli PRIVATE SkyMAPF::core)
target_compile_features(skymapf_cli PRIVATE cxx_std_17)
set_target_properties(skymapf_cli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

skymapf_apply_warnings(skymapf_cli)
