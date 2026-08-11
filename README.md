# SkyMAPF

SkyMAPF is a C++17 project for modeling, generating, solving, and benchmarking
MAPF (Multi-Agent Path Finding) instances. The repository currently contains a
reusable core library, a framework-owned single-run benchmark pipeline,
baseline solvers, small C++ examples, a CLI skeleton, tests, and a pure-Python
solver SDK with optional native bindings built by pybind11/scikit-build-core.

The first interoperability profile is defined under `protocol/v1/`. It
separates a portable problem, a solver-owned raw result, and a framework-owned
validated run record so C++, Python, and external-process solvers can share the
same evaluation contract.

Current status: `0.0.1-beta`. The project is public but still in early beta;
APIs, data formats, and Python bindings may change before a stable release.

## Repository Layout

```text
src/core/          Core C++ library and public headers
src/cli/           CLI executable based on CLI11
example/           Example programs for version output and scenario IO
test/              Catch2 tests
bindings/python/   Python extension module
python/skymapf/     Pure-Python protocol and solver SDK
protocol/v1/       Language-neutral benchmark protocol schemas
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

## Install and consume the C++ SDK

Install the core library, public headers, CMake package metadata, and Protocol
v1 resources into a chosen prefix:

```bash
cmake -S . -B build-sdk \
  -DSKYMAPF_BUILD_CLI=OFF \
  -DSKYMAPF_BUILD_TESTS=OFF \
  -DSKYMAPF_BUILD_EXAMPLES=OFF \
  -DSKYMAPF_BUILD_PYTHON_BINDINGS=OFF
cmake --build build-sdk -j
cmake --install build-sdk --prefix /path/to/skymapf-sdk
```

An independent CMake project can then consume the installed SDK:

```cmake
find_package(SkyMAPF 0.0.1 CONFIG REQUIRED)

add_executable(my_solver main.cpp)
target_link_libraries(my_solver PRIVATE SkyMAPF::core)
```

Configure that project with
`-DCMAKE_PREFIX_PATH=/path/to/skymapf-sdk`. The package exports
`SkyMAPF_PROTOCOL_DIR`, pointing to the installed language-neutral schemas and
specification. `nlohmann_json` is a public C++ dependency and must be available
to `find_package` in the consumer environment.

Build a wheel containing the Python SDK and optional native `_core` extension:

```bash
python -m pip install build
python -m build --wheel
```

External solver authors can also use the source-tree SDK without compiling the
extension. A minimal solver is:

```python
from skymapf import AgentPath, RawSolveResult, solver

@solver("my-cbs", "0.1")
def solve(problem, options):
    paths = run_my_algorithm(problem, options)
    return RawSolveResult.success([
        AgentPath.from_cells(agent_id, cells)
        for agent_id, cells in paths.items()
    ])

if __name__ == "__main__":
    raise SystemExit(solve.main())
```

The application reads one solve request from stdin, writes one raw result to
stdout, and maps unsupported instances or solver exceptions to explicit
protocol statuses. See `doc/python_sdk.md` for the API and execution contract.

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

## Editor Setup

For clangd-based C++ completion and diagnostics, configure CMake once so the
project has `build/compile_commands.json`:

```bash
cmake -S . -B build -DSKYMAPF_BUILD_PYTHON_BINDINGS=OFF
```

The repository includes `.clangd`, which points clangd to the `build`
compilation database. If VS Code still reports include errors from the Microsoft
C/C++ extension, use the clangd extension for IntelliSense or point the C/C++
extension at `build/compile_commands.json`.

## Useful Build Options

| Option | Default | Meaning |
| --- | --- | --- |
| `SKYMAPF_BUILD_CLI` | `ON` | Build `build/bin/skymapf` |
| `SKYMAPF_BUILD_TESTS` | `ON` | Build Catch2 tests and enable CTest |
| `SKYMAPF_BUILD_EXAMPLES` | `ON` | Build example executables |
| `SKYMAPF_BUILD_PYTHON_BINDINGS` | `ON` | Build the Python module |
| `SKYMAPF_USE_SYSTEM_DEPS` | `ON` | Prefer locally installed dependencies |
| `SKYMAPF_WARNINGS_AS_ERRORS` | `OFF` | Treat warnings as errors |
| `SKYMAPF_INSTALL_CPP_SDK` | `ON` | Install C++ targets, headers, and protocol resources |

## Examples

After building examples, binaries are placed under `build/bin/examples`.

```bash
./build/bin/examples/example_1_helloworld__main
./build/bin/examples/example_2_CreateScenario__main
./build/bin/examples/example_3_ReadScenario__main data <scenario_folder_name>
./build/bin/examples/example_5_BenchmarkCBS__main
```

`example/2_CreateScenario` writes a generated scenario under `data/`, and
`example/3_ReadScenario` reads and validates a scenario folder.
`example/5_BenchmarkCBS` runs a two-agent swap through Basic CBS and the
framework-owned validation and metric pipeline. The initial benchmark semantics
are documented in `doc/benchmark_semantics_v1.md`.

## MovingAI compatibility

`skymapf::io::MovingAIIO` imports standard MovingAI `.map` and `.scen` files.
It converts selected scenario prefixes into the unified `InstanceModel`; the
first `k` scenario rows form a `k`-agent benchmark instance. Imported maps use
four-neighbor movement plus wait actions under the `movingai_standard_v1`
semantics profile.

The versioned cross-language contract is described in `protocol/v1/README.md`.
Solvers that cannot implement the requested profile should report
`unsupported`, rather than silently changing its semantics.

## External solver integration

`skymapf::solver::ExternalProcessSolver` connects an arbitrary executable to
the same benchmark runner using Protocol v1 JSON over stdin/stdout. On POSIX
platforms it enforces a hard wall-time limit, isolates solver logs on stderr,
limits captured output, maps process failures to standard statuses, and sends
successful paths through the framework validator and canonical metric pipeline.

The executable contract is documented in `doc/external_solver_protocol_v1.md`;
the reference Python implementation is documented in `doc/python_sdk.md`.

## License

SkyMAPF is distributed under the MIT License. See `LICENSE`.
