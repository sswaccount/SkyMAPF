file(GLOB_RECURSE SKYMAPF_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
find_package(Python3 COMPONENTS Interpreter QUIET)
list(FILTER SKYMAPF_TEST_SOURCES EXCLUDE REGEX "/fixtures/")
list(FILTER SKYMAPF_TEST_SOURCES EXCLUDE REGEX "/consumer/")

add_executable(skymapf_external_solver_fixture
    "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/external_solver_fixture.cpp"
)
target_link_libraries(skymapf_external_solver_fixture PRIVATE nlohmann_json::nlohmann_json)
target_compile_features(skymapf_external_solver_fixture PRIVATE cxx_std_17)
skymapf_apply_warnings(skymapf_external_solver_fixture)
set_target_properties(skymapf_external_solver_fixture PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests"
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

    if(target_name STREQUAL "test_solver__external_process_solver_test")
        add_dependencies(${target_name} skymapf_external_solver_fixture)
        target_compile_definitions(${target_name} PRIVATE
            SKYMAPF_EXTERNAL_SOLVER_FIXTURE="$<TARGET_FILE:skymapf_external_solver_fixture>"
        )
        if(Python3_Interpreter_FOUND)
            target_compile_definitions(${target_name} PRIVATE
                SKYMAPF_PYTHON_EXECUTABLE="${Python3_EXECUTABLE}"
                SKYMAPF_PYTHON_SOLVER_FIXTURE="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/python_solver_fixture.py"
            )
            set_tests_properties(${target_name} PROPERTIES
                ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}/python;PYTHONDONTWRITEBYTECODE=1"
            )
        endif()
    endif()
endforeach()

if(Python3_Interpreter_FOUND)
    add_test(
        NAME test_python__sdk
        COMMAND "${Python3_EXECUTABLE}" -m unittest discover
            -s "${PROJECT_SOURCE_DIR}/python/tests"
            -v
    )
    set_tests_properties(test_python__sdk PROPERTIES
        ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}/python;PYTHONDONTWRITEBYTECODE=1"
    )
endif()

if(SKYMAPF_INSTALL_CPP_SDK)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/installed_consumer_test.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/installed_consumer_test.cmake"
        @ONLY
    )
    add_test(
        NAME test_cmake__installed_consumer
        COMMAND "${CMAKE_COMMAND}"
            -P "${CMAKE_CURRENT_BINARY_DIR}/installed_consumer_test.cmake"
    )
endif()
