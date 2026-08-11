# SkyMAPF Python SDK

The Python SDK is the reference authoring surface for external Protocol v1
solvers. Its protocol types use only the Python standard library. Importing
`skymapf` therefore works even when the optional native `skymapf._core`
extension is unavailable; `skymapf.HAS_CORE` reports whether it was loaded.

## Solver application

```python
from skymapf import AgentPath, RawSolveResult, UnsupportedProblem, solver

@solver("example", "1.0", commit="git-sha")
def solve(problem, options):
    if problem.objective != "sum_of_costs":
        raise UnsupportedProblem("only sum-of-costs is implemented")

    paths = algorithm(problem, options.time_limit_ms, options.random_seed)
    return RawSolveResult.success(
        [AgentPath.from_cells(agent_id, cells) for agent_id, cells in paths.items()],
        statistics={"expanded_nodes": 100},
    )

if __name__ == "__main__":
    raise SystemExit(solve.main())
```

`SolverApplication.main()` reads one JSON request from stdin and writes exactly
one compact JSON result followed by a newline to stdout. Algorithm logs belong
on stderr. Invalid input exits with code 2 without writing stdout. An unexpected
algorithm exception is logged to stderr and becomes a protocol `error` result;
`UnsupportedProblem` becomes `unsupported`.

## Data model

- `Problem` contains the immutable id, objective, rectangular world, blocked
  cells, agents, and open metadata. `in_bounds`, `is_walkable`, `neighbors`,
  and `agent_by_id` are convenience helpers.
- `SolveOptions` contains the runner-requested time limit, random seed, and node
  limit. Zero means that the runner did not request that limit.
- `AgentPath.from_cells(agent_id, cells)` builds a consecutive-timestep path.
- `RawSolveResult` provides `success`, `infeasible`, `timeout`, `unsupported`,
  and `error` constructors.

The SDK validates the closed Protocol v1 structure, coordinate and identifier
types, world bounds, blocked starts or goals, duplicate ids, JSON-compatible
metadata, and result status/path consistency. It intentionally does not decide
whether a returned plan is collision-free: the framework validator remains the
authoritative judge and recomputes canonical metrics.

## Local protocol invocation

With a source checkout:

```bash
PYTHONPATH=python python path/to/solver.py \
  < test/data/protocol/v1/valid/solve-request.json
```

The C++ `ExternalProcessSolver` uses the same invocation contract, so the same
executable can be tested locally and admitted to a benchmark runner without an
in-process language binding.
