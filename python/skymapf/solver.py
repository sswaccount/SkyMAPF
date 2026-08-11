"""Small application wrapper for implementing protocol-compatible solvers."""

from __future__ import annotations

import sys
import traceback
from dataclasses import dataclass
from typing import Any, Callable, Mapping, TextIO

from .protocol import (
    Problem,
    ProtocolError,
    RawSolveResult,
    SolveOptions,
    SolverMetadata,
    read_request,
    write_result,
)


class UnsupportedProblem(RuntimeError):
    """A solver can raise this when it cannot preserve requested semantics."""


SolveFunction = Callable[[Problem, SolveOptions], RawSolveResult]


@dataclass(frozen=True)
class SolverApplication:
    solve: SolveFunction
    metadata: SolverMetadata

    def __call__(self, problem: Problem, options: SolveOptions) -> RawSolveResult:
        result = self.solve(problem, options)
        if not isinstance(result, RawSolveResult):
            raise TypeError("solver function must return RawSolveResult")
        return result.bound_to(problem.problem_id, self.metadata)

    def main(
        self,
        stdin: TextIO = sys.stdin,
        stdout: TextIO = sys.stdout,
        stderr: TextIO = sys.stderr,
    ) -> int:
        try:
            request = read_request(stdin)
        except ProtocolError as exc:
            stderr.write(f"invalid SkyMAPF request: {exc}\n")
            stderr.flush()
            return 2

        try:
            result = self(request.problem, request.options)
        except UnsupportedProblem as exc:
            result = RawSolveResult.unsupported(str(exc)).bound_to(
                request.problem.problem_id, self.metadata
            )
        except Exception as exc:  # Solver failures become protocol-level errors.
            traceback.print_exc(file=stderr)
            result = RawSolveResult.error(
                f"{type(exc).__name__}: {exc}"
            ).bound_to(request.problem.problem_id, self.metadata)

        try:
            write_result(result, stdout)
        except (ProtocolError, TypeError, ValueError) as exc:
            stderr.write(f"invalid SkyMAPF result: {exc}\n")
            stderr.flush()
            return 3
        return 0


def solver(
    name: str,
    version: str,
    *,
    commit: str = "",
    configuration: Mapping[str, Any] | None = None,
) -> Callable[[SolveFunction], SolverApplication]:
    """Decorate a solve function and expose it as a protocol application."""
    if not name or not version:
        raise ValueError("solver name and version must be non-empty")
    metadata = SolverMetadata(name, version, commit, configuration or {})

    def decorate(function: SolveFunction) -> SolverApplication:
        return SolverApplication(function, metadata)

    return decorate
