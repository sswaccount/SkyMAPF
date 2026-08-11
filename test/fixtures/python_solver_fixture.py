#!/usr/bin/env python3
"""Protocol-compatible Python solver used by the C++ external runner test."""

from skymapf import AgentPath, RawSolveResult, solver


@solver("python_fixture", "1.0")
def solve(problem, options):
    del options
    return RawSolveResult.success(
        [
            AgentPath.from_cells(1, [(0, 0), (0, 1), (1, 1), (1, 0)]),
            AgentPath.from_cells(2, [(1, 0), (0, 0)]),
        ],
        statistics={"expanded_nodes": 13, "generated_nodes": 17},
    )


if __name__ == "__main__":
    raise SystemExit(solve.main())
