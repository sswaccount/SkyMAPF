# SkyMAPF Standard MAPF Benchmark Semantics v1

Protocol semantics identifier: `movingai_standard_v1`. The portable request,
raw solver response, and framework-owned run record are defined by the JSON
Schemas in `protocol/v1/`.

This document fixes the minimum problem semantics used by the first SkyMAPF
benchmark pipeline. Algorithm families may support additional features, but
they must not silently reinterpret an instance.

## Instance

- One instance contains one world and one task.
- Every task entry identifies one agent and exactly two checkpoints: start and
  goal.
- Every agent starts at timestep 0.
- Agents may move to a world neighbor or wait for one timestep.
- After reaching its goal, an agent remains there and continues to occupy the
  goal cell.

Waypoint sequences, delayed starts, and disappear-at-goal behavior are outside
Standard MAPF v1. A solver that does not implement such extensions must return
`SolveStatus::Unsupported` instead of ignoring them.

## Valid solution

A valid solution contains exactly one non-empty path for every task agent. Each
path must:

- start at the assigned start cell at timestep 0;
- end at the assigned goal cell;
- contain only walkable cells;
- use only wait actions or world edges;
- contain no vertex conflict with another path; and
- contain no opposite-direction edge conflict with another path.

The framework validator is the authority for solution validity. A solver's
success status alone does not make a solution valid.

## Canonical metrics

- `sum_of_costs`: sum of `path_length - 1` over all agents.
- `makespan`: maximum goal-arrival timestep over all agents.
- `wall_time_ms`: elapsed steady-clock time measured around the solver call by
  the benchmark runner.

Solver-specific counters such as expanded and generated nodes are self-reported
diagnostics. Canonical solution metrics are recomputed by the benchmark runner.

## Current execution boundary

The v1 runner supports two execution boundaries:

- in-process C++ solvers, where time limits remain cooperative and exceptions
  are converted to errors; and
- external solvers on POSIX systems, where a child process group is isolated,
  captured output is limited, and wall-time expiration is enforced by hard
  termination.

Run records can now be serialized through `ProtocolV1Codec`. Peak-memory
measurement, platform-neutral hard isolation, batch experiment manifests, and
official result storage remain future benchmark infrastructure.
