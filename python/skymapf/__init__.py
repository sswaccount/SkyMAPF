"""SkyMAPF SDK for protocol-compatible MAPF solvers."""

from .protocol import (
    PROBLEM_PROTOCOL_V1,
    RAW_SOLVE_RESULT_PROTOCOL_V1,
    SOLVE_REQUEST_PROTOCOL_V1,
    STANDARD_MAPF_SEMANTICS_V1,
    Agent,
    AgentPath,
    Problem,
    ProtocolError,
    RawSolveResult,
    SolveOptions,
    SolveRequest,
    SolverMetadata,
    read_request,
    write_result,
)
from .solver import SolverApplication, UnsupportedProblem, solver

try:
    from . import _core
except ImportError:
    _core = None

HAS_CORE = _core is not None
__version__ = _core.__version__ if HAS_CORE else "0.0.1b0"

__all__ = [
    "Agent",
    "AgentPath",
    "HAS_CORE",
    "PROBLEM_PROTOCOL_V1",
    "Problem",
    "ProtocolError",
    "RAW_SOLVE_RESULT_PROTOCOL_V1",
    "RawSolveResult",
    "SOLVE_REQUEST_PROTOCOL_V1",
    "STANDARD_MAPF_SEMANTICS_V1",
    "SolveOptions",
    "SolveRequest",
    "SolverApplication",
    "SolverMetadata",
    "UnsupportedProblem",
    "read_request",
    "solver",
    "write_result",
]
