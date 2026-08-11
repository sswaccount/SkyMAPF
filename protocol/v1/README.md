# SkyMAPF Protocol v1-draft

SkyMAPF Protocol is the language-neutral boundary between problem producers,
solver implementations, benchmark runners, and result consumers. The schemas in
this directory use JSON Schema Draft 2020-12.

The initial profile is intentionally narrow:

- problem type: `standard_mapf`;
- semantics: `movingai_standard_v1`;
- discrete four-neighbor movement with wait actions;
- all agents start at timestep zero;
- agents remain at their goals;
- vertex and opposite-direction edge conflicts are forbidden; and
- objective is either sum of costs or makespan.

Extensions must use a different versioned semantics identifier. A solver must
return `unsupported` rather than silently ignore problem features.

## Documents

- `spec.md`: normative semantics, status rules, compatibility, and fingerprint
  policy.
- `problem.schema.json`: portable problem request.
- `solve-request.schema.json`: one problem plus reproducible runtime options,
  sent to an external solver on standard input.
- `raw-solve-result.schema.json`: solver-owned status, plan, and diagnostics.
- `run-record.schema.json`: framework-owned validation, canonical metrics, and
  provenance for one execution.
- `CHANGELOG.md`: protocol-only changes made before and after freezing v1.

The C++ API may evolve during beta. These serialized schemas are the intended
cross-language contract for future C++, Python, and external-process SDKs.

An external solver invocation consumes exactly one `solve-request.v1` JSON
document from standard input and writes exactly one `raw-solve-result.v1` JSON
document to standard output. Logs belong on standard error. The framework does
not trust solver-reported solution metrics; it validates paths and recomputes
canonical metrics after the process exits.

The documents currently have **draft** stability. Implementations should use
the exact protocol identifiers above, but the schemas may still receive
breaking corrections until the repository explicitly marks v1 frozen. Golden
fixtures under `test/data/protocol/v1/` define accepted and rejected examples.
