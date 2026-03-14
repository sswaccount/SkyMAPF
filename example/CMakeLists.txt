# ---------------------------------------------------------------------------
# example/CMakeLists.txt
# Build one executable per example source file.
# ---------------------------------------------------------------------------

file(GLOB_RECURSE SKYMAPF_EXAMPLE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
)

foreach(example_src IN LISTS SKYMAPF_EXAMPLE_SOURCES)
    file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_SOURCE_DIR}" "${example_src}")
    string(REGEX REPLACE "[/\\\\]" "__" target_suffix "${rel_path}")
    string(REGEX REPLACE "\\.[^.]+$" "" target_suffix "${target_suffix}")
    set(target_name "example_${target_suffix}")

    add_executable(${target_name} "${example_src}")
    target_link_libraries(${target_name} PRIVATE SkyMAPF::core)
    target_compile_features(${target_name} PRIVATE cxx_std_17)
    skymapf_apply_warnings(${target_name})

    # Keep example binaries grouped under build/bin/examples.
    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/examples"
    )
endforeach()
