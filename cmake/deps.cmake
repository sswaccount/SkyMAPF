include(FetchContent)

option(SKYMAPF_USE_SYSTEM_DEPS "Prefer preinstalled dependencies via find_package" ON)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Disable automatic FetchContent updates" FORCE)

if(SKYMAPF_USE_SYSTEM_DEPS)
  find_package(nlohmann_json CONFIG QUIET)
endif()

if(NOT nlohmann_json_FOUND)
  FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
  )
  FetchContent_MakeAvailable(nlohmann_json)
endif()

if(SKYMAPF_BUILD_TESTS)
  if(SKYMAPF_USE_SYSTEM_DEPS)
    find_package(Catch2 3 CONFIG QUIET)
  endif()

  if(NOT Catch2_FOUND)
    FetchContent_Declare(
      Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)
  endif()
endif()

if(SKYMAPF_BUILD_PYTHON_BINDINGS)
  if(SKYMAPF_USE_SYSTEM_DEPS)
    find_package(pybind11 CONFIG QUIET)
  endif()

  if(NOT pybind11_FOUND)
    FetchContent_Declare(
      pybind11
      GIT_REPOSITORY https://github.com/pybind/pybind11.git
      GIT_TAG v2.13.6
    )
    FetchContent_MakeAvailable(pybind11)
  endif()
endif()
