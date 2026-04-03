file(GLOB_RECURSE SKYMAPF_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

message(STATUS "SKYMAPF_TEST_SOURCES=${SKYMAPF_TEST_SOURCES}")

foreach(test_src IN LISTS SKYMAPF_TEST_SOURCES)
    file(RELATIVE_PATH rel_path "${CMAKE_CURRENT_SOURCE_DIR}" "${test_src}")
    string(REGEX REPLACE "[/\\\\]" "__" target_suffix "${rel_path}")
    string(REGEX REPLACE "\\.[^.]+$" "" target_suffix "${target_suffix}")
    set(target_name "test_${target_suffix}")

    add_executable(${target_name} "${test_src}")
    target_link_libraries(${target_name} PRIVATE SkyMAPF::core Catch2::Catch2WithMain)
    target_compile_features(${target_name} PRIVATE cxx_std_17)
    skymapf_apply_warnings(${target_name})

    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests"
    )

    add_test(NAME ${target_name} COMMAND ${target_name})
endforeach()