# SkyMAPF

SkyMAPF is a C++17 project for modeling and generating MAPF (Multi-Agent Path
Finding) datasets. The repository currently contains a reusable core library,
small C++ examples, a CLI skeleton, tests, and Python bindings built with
pybind11/scikit-build-core.

Current status: `0.0.1-beta`. The project is public but still in early beta;
APIs, data formats, and Python bindings may change before a stable release.

## Repository Layout

```text
src/core/          Core C++ library and public headers
src/cli/           CLI executable based on CLI11
example/           Example programs for version output and scenario IO
test/              Catch2 tests
bindings/python/   Python extension module
cmake/             Dependency setup
doc/               Comment style notes
```

The main public C++ include is:

```cpp
#include <skymapf/skymapf.hpp>
```

## Requirements

- CMake 3.15 or newer
- A C++17 compiler
- Git access to GitHub, unless dependencies are already installed locally
- Optional Python build requirements: Python 3.10+, `scikit-build-core`,
  `pybind11`

The CMake build uses these third-party dependencies:

- `nlohmann_json`
- `CLI11` when `SKYMAPF_BUILD_CLI=ON`
- `Catch2` when `SKYMAPF_BUILD_TESTS=ON`
- `pybind11` when `SKYMAPF_BUILD_PYTHON_BINDINGS=ON`

By default, CMake first tries `find_package(... CONFIG QUIET)`. If a dependency
is not found, it falls back to `FetchContent` and clones the dependency from
GitHub.

## Build

Default build:

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Build only the core library and examples:

```bash
cmake -S . -B build-core \
  -DSKYMAPF_BUILD_CLI=OFF \
  -DSKYMAPF_BUILD_TESTS=OFF \
  -DSKYMAPF_BUILD_PYTHON_BINDINGS=OFF
cmake --build build-core -j
```

Build the Python extension:

```bash
python -m pip install build
python -m build --wheel
```

## Dependency Fetching

If required dependencies are not available locally, CMake falls back to
`FetchContent` and clones them from GitHub. In restricted or offline
environments, configure may fail before compiling project source code. This is
a dependency/network setup issue, not necessarily a C++ source compilation
error. The default options enable the CLI, tests, examples, and Python bindings,
so CMake may need CLI11, Catch2, pybind11, and nlohmann_json.

Practical fixes:

1. Make GitHub access work in the build environment, then run the default CMake
   commands again.
2. Install dependencies through the system/package manager so `find_package`
   can locate them.
3. Disable optional targets that are not needed for the current build, for
   example:

```bash
cmake -S . -B build \
  -DSKYMAPF_BUILD_CLI=OFF \
  -DSKYMAPF_BUILD_TESTS=OFF \
  -DSKYMAPF_BUILD_PYTHON_BINDINGS=OFF
```

If examples remain enabled, `nlohmann_json` is still required because the core
library links it publicly.

## Development CI

The development workflow runs on pushes to `dev`, pull requests targeting
`dev` or `main`, and manual dispatch. It currently verifies the C++ core, CLI,
tests, and examples. Python bindings are experimental and are not part of the
required development CI yet.

## Useful Build Options

| Option | Default | Meaning |
| --- | --- | --- |
| `SKYMAPF_BUILD_CLI` | `ON` | Build `build/bin/skymapf` |
| `SKYMAPF_BUILD_TESTS` | `ON` | Build Catch2 tests and enable CTest |
| `SKYMAPF_BUILD_EXAMPLES` | `ON` | Build example executables |
| `SKYMAPF_BUILD_PYTHON_BINDINGS` | `ON` | Build the Python module |
| `SKYMAPF_USE_SYSTEM_DEPS` | `ON` | Prefer locally installed dependencies |
| `SKYMAPF_WARNINGS_AS_ERRORS` | `OFF` | Treat warnings as errors |

## Examples

After building examples, binaries are placed under `build/bin/examples`.

```bash
./build/bin/examples/example_1_helloworld__main
./build/bin/examples/example_2_CreateScenario__main
./build/bin/examples/example_3_ReadScenario__main data <scenario_folder_name>
```

`example/2_CreateScenario` writes a generated scenario under `data/`, and
`example/3_ReadScenario` reads and validates a scenario folder.

## License

SkyMAPF is distributed under the MIT License. See `LICENSE`.
