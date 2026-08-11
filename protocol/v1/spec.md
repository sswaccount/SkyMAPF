# SkyMAPF Protocol v1-draft Specification

This document is normative for semantics. The JSON Schemas in this directory
are normative for serialized structure. The terms MUST, MUST NOT, SHOULD, and
MAY describe compatibility requirements.

## Scope

Protocol v1 has one problem profile: `standard_mapf` with semantics identifier
`movingai_standard_v1`.

- The world MUST be a finite two-dimensional rectangular grid.
- Coordinates are zero-based `[x, y]`, with `[0, 0]` at the top-left.
- One action advances one timestep and is either wait or a cardinal move to a
  walkable cell.
- Every agent starts at timestep zero and remains at its goal indefinitely.
- Two agents MUST NOT occupy one cell at the same timestep.
- Two agents MUST NOT traverse one edge in opposite directions in one timestep.
- A problem objective is either `sum_of_costs` or `makespan`.

Diagonal movement, weighted edges, delayed starts, waypoints, disappear-at-goal
behavior, continuous time, and 3D worlds are not v1 features. A solver MUST
return `unsupported` when it cannot preserve requested semantics.

## Protocol documents

One external invocation consumes one `skymapf.solve-request.v1` document and
produces one `skymapf.raw-solve-result.v1` document.

The solve request contains:

- one immutable problem;
- a wall-time limit in milliseconds, where zero means no runner-requested
  limit;
- a random seed; and
- a node limit, where zero means no runner-requested node limit.

The raw result is solver-owned and untrusted. The framework-owned
`skymapf.run-record.v1` is the publishable outcome after validation and metric
recomputation.

Run-record validation contains a `performed` flag. When no candidate plan was
validated (for example timeout or unsupported), `performed` and `valid` are
both false and `errors` is empty. This is distinct from a performed validation
that rejected a plan.

Unknown properties are rejected wherever a Schema uses
`additionalProperties: false`. The deliberately open `metadata`,
`configuration`, `statistics`, and `environment` objects MUST NOT change core
problem semantics or canonical metrics.

## Raw result status rules

- `success`: `paths` MUST contain a non-empty path for every problem agent and
  MUST NOT contain other or duplicate agents.
- `infeasible`: the solver completed and claims no solution exists; `paths`
  MUST be empty.
- `timeout`: a time limit stopped the search; `paths` MUST be empty.
- `unsupported`: the solver does not implement the requested semantics;
  `paths` MUST be empty.
- `error`: execution or solver logic failed; `paths` MUST be empty.

Each success path is a sequence of coordinates at consecutive timesteps
starting at zero. The final coordinate is occupied for all later timesteps.
Logs MUST go to stderr; stdout contains only the JSON result.

## Validation and canonical metrics

The benchmark framework, not the solver, is authoritative. It validates agent
coverage, starts, goals, walkability, moves, vertex conflicts, and edge
conflicts. A solver `success` rejected by validation becomes
`invalid_solution` in the run record.

Canonical metrics are:

- `sum_of_costs`: sum of `(path coordinate count - 1)` over all agents;
- `makespan`: maximum `(path coordinate count - 1)` over all agents; and
- `wall_time_ms`: framework-observed steady-clock duration around execution.

Expanded and generated node counts are solver-reported diagnostics and are not
trusted measures of solution validity or quality.

## Instance fingerprint

The C++ reference implementation currently emits
`fnv1a64:<16 lowercase hex digits>`. It hashes canonical compact JSON containing
only `problem_type`, `semantics`, `world`, and `agents`.

- Blocked cells are row-major.
- Agents are sorted by numeric id.
- Display metadata, problem id, objective, and insertion order are excluded.

FNV-1a is a deterministic cache and identity fingerprint, not a cryptographic
integrity signature. A future protocol may introduce a stronger algorithm with
an explicit prefix without reinterpreting existing fingerprints.

## Compatibility and freezing

During `v1-draft`, breaking corrections are recorded in `CHANGELOG.md`. Once v1
is frozen:

- existing required fields and meanings will not change;
- new optional core fields require a new protocol identifier if old strict
  readers would reject them;
- new problem semantics require a new semantics identifier; and
- incompatible document structure requires a new protocol major version.

Conformance is determined by the Schemas, this specification, and the published
Golden Fixtures. Passing JSON Schema validation alone is necessary but not
sufficient; semantic validation by the framework is also required.
