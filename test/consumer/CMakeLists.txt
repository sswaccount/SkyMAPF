cmake_minimum_required(VERSION 3.15)

project(SkyMAPFInstalledConsumer LANGUAGES CXX)

find_package(SkyMAPF 0.0.1 CONFIG REQUIRED)

add_executable(skymapf_sdk_consumer main.cpp)
target_link_libraries(skymapf_sdk_consumer PRIVATE SkyMAPF::core)
target_compile_features(skymapf_sdk_consumer PRIVATE cxx_std_17)

set_target_properties(skymapf_sdk_consumer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(NOT EXISTS "${SkyMAPF_PROTOCOL_DIR}/v1/problem.schema.json")
    message(FATAL_ERROR "Installed SkyMAPF protocol resources were not found")
endif()
