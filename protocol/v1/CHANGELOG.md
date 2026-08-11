# Protocol v1 Changelog

## v1-draft — 2026-08-11

- Defined Standard MAPF problem, solve request, raw result, and run record.
- Fixed `movingai_standard_v1` movement, conflict, and goal-occupancy semantics.
- Required non-empty paths for `success` and empty paths for all other statuses.
- Defined strict unknown-field handling at closed Schema boundaries.
- Added framework-owned validation errors, canonical metrics, diagnostics,
  resources, and provenance to RunRecord.
- Distinguished validation that passed, validation that failed, and validation
  that was not performed.
- Defined the canonical `fnv1a64` semantic-instance fingerprint.
- Published accepted and rejected Golden Fixtures.
- Split search diagnostics into high-level and low-level node counters while
  retaining aggregate expanded/generated counters.
