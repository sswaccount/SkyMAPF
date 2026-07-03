# Contributing

SkyMAPF is in early beta development. The public API, data formats, and Python
bindings may still change.

## Branches

- `main` is intended to stay buildable and reviewable.
- `dev` is the active development branch.
- Open pull requests against `main` unless a maintainer asks otherwise.

## Local Build

Run the same core checks used by the development CI:

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DSKYMAPF_BUILD_PYTHON_BINDINGS=OFF
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/bin/skymapf --version
./build/bin/skymapf --help
./build/bin/examples/example_1_helloworld__main
./build/bin/examples/example_2_CreateScenario__main
```

Python bindings are present but not part of the required development CI yet.

## Style

Use the existing CMake and C++ style in the surrounding files. Comment style
notes are available under `doc/`.
