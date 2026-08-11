# External Solver Protocol v1

SkyMAPF can evaluate a solver implemented in any language as an isolated
executable. The executable boundary is deliberately small:

1. read exactly one `skymapf.solve-request.v1` JSON document from stdin;
2. write exactly one `skymapf.raw-solve-result.v1` JSON document to stdout;
3. write human-readable logs only to stderr; and
4. exit with code zero after producing a response.

The schemas are stored in `protocol/v1/`. The request contains the Standard
MAPF problem and the runner-owned time limit, random seed, and node limit. Paths
in the response use `[x, y]` coordinates at consecutive timesteps starting at
zero. Under `movingai_standard_v1`, agents wait at their final coordinate after
the listed path ends.

The framework treats stdout as untrusted input. It checks the protocol version
and problem identity, converts coordinates, rejects malformed or duplicate
paths, validates starts, goals, moves and conflicts, then recomputes sum of
costs and makespan. Solver-reported search counters remain diagnostics.

The complete normative rules are in `protocol/v1/spec.md`. Files under
`test/data/protocol/v1/valid/` are accepted Golden Fixtures; files under
`test/data/protocol/v1/invalid/` MUST be rejected by compatible adapters.

## C++ runner integration

```cpp
skymapf::solver::ExternalProcessSolverConfig config;
config.executable = "/path/to/my-solver";
config.arguments = {"--algorithm", "my-cbs"};
config.solver_info.name = "my-cbs";
config.solver_info.version = "1.0";

skymapf::solver::ExternalProcessSolver solver(std::move(config));
auto record = skymapf::benchmark::BenchmarkRunner::run(
    solver,
    instance,
    skymapf::solver::SolveOptions{30000, 42, 0}
);
```

On POSIX systems the adapter runs the solver in a child process, imposes an
output-size limit, and kills the child process group when the wall-time limit
expires. Non-zero exits, signals, malformed JSON, identity mismatches, and
invalid solutions become explicit benchmark outcomes.

## Python SDK boundary

The reference `skymapf` Python SDK preserves this wire contract and exposes:

- `SolveRequest.from_stream()` and `read_request()` for validated parsing;
- `Problem` and `Agent` typed views over the Standard MAPF instance;
- `RawSolveResult.success(paths, statistics=...)` and status constructors; and
- the `@solver(...)` application wrapper for a single protocol-compliant
  stdin/stdout invocation.

The protocol layer is pure Python and does not load the C++ extension inside a
submitted solver process. The optional `skymapf._core` extension can separately
expose high-performance models, algorithms, and in-process benchmark analysis.
