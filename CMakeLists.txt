cmake_minimum_required(VERSION 3.20)

# ---------------------------------------------------------------------------
# Project metadata and version policy
# ---------------------------------------------------------------------------
project(SkyMAPF VERSION 0.1.0 LANGUAGES CXX)

# Keep numeric version in project(...), and manage prerelease tag separately.
set(SKYMAPF_VERSION_SUFFIX "-beta")
set(SKYMAPF_FULL_VERSION "${PROJECT_VERSION}${SKYMAPF_VERSION_SUFFIX}")

# ---------------------------------------------------------------------------
# Global build settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default build type for single-config generators.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

# Project switches.
option(SKYMAPF_BUILD_CLI "Build CLI executable under src/cli." ON)
option(SKYMAPF_BUILD_TESTS "Build tests under test/." ON)
option(SKYMAPF_BUILD_EXAMPLES "Build examples under example/." OFF)
option(SKYMAPF_BUILD_PYTHON_BINDINGS "Build Python bindings under bindings/python." ON)
option(SKYMAPF_WARNINGS_AS_ERRORS "Treat compiler warnings as errors." OFF)

# Keep outputs tidy.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(cmake/deps.cmake)

# ---------------------------------------------------------------------------
# Generate public version header from CMake project version.
# ---------------------------------------------------------------------------
set(SKYMAPF_GENERATED_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${SKYMAPF_GENERATED_INCLUDE_DIR}/skymapf")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/include/skymapf/version.hpp.in.cmake"
    "${SKYMAPF_GENERATED_INCLUDE_DIR}/skymapf/version.hpp"
    @ONLY
)

# ---------------------------------------------------------------------------
# Shared warning helper (used by core/cli/example/test)
# ---------------------------------------------------------------------------
function(skymapf_apply_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive- /Zc:__cplusplus)
        if(SKYMAPF_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE /WX)
        endif()
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
        if(SKYMAPF_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE -Werror)
        endif()
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Core and CLI are delegated to their own CMakeLists.txt
# ---------------------------------------------------------------------------
add_subdirectory(src/core)

if(SKYMAPF_BUILD_CLI)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/cli/CMakeLists.txt")
        add_subdirectory(src/cli)
    else()
        message(STATUS "SKYMAPF_BUILD_CLI=ON, but src/cli/CMakeLists.txt was not found.")
    endif()
endif()

# ---------------------------------------------------------------------------
# examples and tests are delegated to their own CMakeLists.txt
# ---------------------------------------------------------------------------
if(SKYMAPF_BUILD_EXAMPLES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/example/CMakeLists.txt")
        add_subdirectory(example)
    else()
        message(STATUS "SKYMAPF_BUILD_EXAMPLES=ON, but example/CMakeLists.txt was not found.")
    endif()
endif()

if(SKYMAPF_BUILD_TESTS)
    enable_testing()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")
        add_subdirectory(test)
    else()
        message(STATUS "SKYMAPF_BUILD_TESTS=ON, but test/CMakeLists.txt was not found.")
    endif()
endif()

# ---------------------------------------------------------------------------
# Python bindings
# ---------------------------------------------------------------------------
if(SKYMAPF_BUILD_PYTHON_BINDINGS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/bindings/python/CMakeLists.txt")
        add_subdirectory(bindings/python)
    else()
        message(STATUS "SKYMAPF_BUILD_PYTHON_BINDINGS=ON, but bindings/python/CMakeLists.txt was not found.")
    endif()
endif()