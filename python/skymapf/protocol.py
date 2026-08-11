"""Pure-Python data model and codec for the SkyMAPF v1 protocol."""

from __future__ import annotations

import json
import sys
from dataclasses import dataclass, field
from typing import Any, Dict, Iterable, Mapping, Sequence, TextIO, Tuple


PROBLEM_PROTOCOL_V1 = "skymapf.problem.v1"
SOLVE_REQUEST_PROTOCOL_V1 = "skymapf.solve-request.v1"
RAW_SOLVE_RESULT_PROTOCOL_V1 = "skymapf.raw-solve-result.v1"
STANDARD_MAPF_SEMANTICS_V1 = "movingai_standard_v1"

Coordinate = Tuple[int, int]
JsonObject = Dict[str, Any]


class ProtocolError(ValueError):
    """Raised when a protocol document is structurally or semantically invalid."""


def _object(value: Any, where: str) -> Mapping[str, Any]:
    if not isinstance(value, dict):
        raise ProtocolError(f"{where} must be an object")
    return value


def _closed(obj: Mapping[str, Any], allowed: Iterable[str], where: str) -> None:
    unknown = set(obj).difference(allowed)
    if unknown:
        names = ", ".join(sorted(unknown))
        raise ProtocolError(f"{where} contains unknown properties: {names}")


def _required(obj: Mapping[str, Any], names: Iterable[str], where: str) -> None:
    missing = set(names).difference(obj)
    if missing:
        fields = ", ".join(sorted(missing))
        raise ProtocolError(f"{where} is missing required properties: {fields}")


def _string(value: Any, where: str, *, nonempty: bool = False) -> str:
    if not isinstance(value, str) or (nonempty and not value):
        qualifier = "a non-empty string" if nonempty else "a string"
        raise ProtocolError(f"{where} must be {qualifier}")
    return value


def _integer(value: Any, where: str, minimum: int = 0) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value < minimum:
        raise ProtocolError(f"{where} must be an integer >= {minimum}")
    return value


def _coordinate(value: Any, where: str) -> Coordinate:
    if not isinstance(value, (list, tuple)) or len(value) != 2:
        raise ProtocolError(f"{where} must be a two-element coordinate")
    return (_integer(value[0], f"{where}[0]"), _integer(value[1], f"{where}[1]"))


def _json_object(value: Any, where: str) -> JsonObject:
    obj = _object(value, where)
    # Round-tripping also rejects values that cannot be represented in JSON.
    try:
        return json.loads(json.dumps(obj, allow_nan=False))
    except (TypeError, ValueError) as exc:
        raise ProtocolError(f"{where} must contain JSON values") from exc


@dataclass(frozen=True)
class Agent:
    id: int
    start: Coordinate
    goal: Coordinate


@dataclass(frozen=True)
class Problem:
    problem_id: str
    objective: str
    width: int
    height: int
    blocked_cells: Tuple[Coordinate, ...]
    agents: Tuple[Agent, ...]
    metadata: Mapping[str, Any] = field(default_factory=dict)

    def in_bounds(self, cell: Coordinate) -> bool:
        return 0 <= cell[0] < self.width and 0 <= cell[1] < self.height

    def is_walkable(self, cell: Coordinate) -> bool:
        return self.in_bounds(cell) and cell not in self.blocked_cells

    def neighbors(self, cell: Coordinate, include_wait: bool = True) -> Tuple[Coordinate, ...]:
        candidates = (
            cell,
            (cell[0] + 1, cell[1]),
            (cell[0] - 1, cell[1]),
            (cell[0], cell[1] + 1),
            (cell[0], cell[1] - 1),
        ) if include_wait else (
            (cell[0] + 1, cell[1]),
            (cell[0] - 1, cell[1]),
            (cell[0], cell[1] + 1),
            (cell[0], cell[1] - 1),
        )
        return tuple(candidate for candidate in candidates if self.is_walkable(candidate))

    def agent_by_id(self, agent_id: int) -> Agent:
        for agent in self.agents:
            if agent.id == agent_id:
                return agent
        raise KeyError(agent_id)


@dataclass(frozen=True)
class SolveOptions:
    time_limit_ms: int
    random_seed: int
    node_limit: int


@dataclass(frozen=True)
class SolveRequest:
    problem: Problem
    options: SolveOptions

    @classmethod
    def from_dict(cls, value: Any) -> "SolveRequest":
        root = _object(value, "solve request")
        _closed(root, {"protocol_version", "problem", "options"}, "solve request")
        _required(root, {"protocol_version", "problem", "options"}, "solve request")
        if root["protocol_version"] != SOLVE_REQUEST_PROTOCOL_V1:
            raise ProtocolError("unsupported solve request protocol_version")
        return cls(_parse_problem(root["problem"]), _parse_options(root["options"]))

    @classmethod
    def from_json(cls, value: str) -> "SolveRequest":
        try:
            document = json.loads(value)
        except json.JSONDecodeError as exc:
            raise ProtocolError(f"invalid JSON: {exc.msg}") from exc
        return cls.from_dict(document)

    @classmethod
    def from_stream(cls, stream: TextIO) -> "SolveRequest":
        return cls.from_json(stream.read())


def _parse_problem(value: Any) -> Problem:
    obj = _object(value, "problem")
    allowed = {
        "protocol_version", "problem_id", "problem_type", "semantics",
        "objective", "world", "agents", "metadata",
    }
    required = allowed.difference({"metadata"})
    _closed(obj, allowed, "problem")
    _required(obj, required, "problem")
    if obj["protocol_version"] != PROBLEM_PROTOCOL_V1:
        raise ProtocolError("unsupported problem protocol_version")
    if obj["problem_type"] != "standard_mapf":
        raise ProtocolError("problem_type must be standard_mapf")
    if obj["semantics"] != STANDARD_MAPF_SEMANTICS_V1:
        raise ProtocolError("unsupported problem semantics")
    objective = obj["objective"]
    if objective not in ("sum_of_costs", "makespan"):
        raise ProtocolError("objective must be sum_of_costs or makespan")

    world = _object(obj["world"], "problem.world")
    world_fields = {"kind", "width", "height", "blocked_cells"}
    _closed(world, world_fields, "problem.world")
    _required(world, world_fields, "problem.world")
    if world["kind"] != "grid_2d":
        raise ProtocolError("problem.world.kind must be grid_2d")
    width = _integer(world["width"], "problem.world.width", 1)
    height = _integer(world["height"], "problem.world.height", 1)
    if not isinstance(world["blocked_cells"], list):
        raise ProtocolError("problem.world.blocked_cells must be an array")
    blocked = tuple(
        _coordinate(cell, f"problem.world.blocked_cells[{index}]")
        for index, cell in enumerate(world["blocked_cells"])
    )
    if len(set(blocked)) != len(blocked):
        raise ProtocolError("problem.world.blocked_cells contains duplicates")
    for cell in blocked:
        if cell[0] >= width or cell[1] >= height:
            raise ProtocolError(f"blocked cell {cell} is outside the world")

    values = obj["agents"]
    if not isinstance(values, list) or not values:
        raise ProtocolError("problem.agents must be a non-empty array")
    agents = []
    ids = set()
    for index, value in enumerate(values):
        where = f"problem.agents[{index}]"
        item = _object(value, where)
        _closed(item, {"id", "start", "goal"}, where)
        _required(item, {"id", "start", "goal"}, where)
        agent = Agent(
            _integer(item["id"], f"{where}.id", 1),
            _coordinate(item["start"], f"{where}.start"),
            _coordinate(item["goal"], f"{where}.goal"),
        )
        if agent.id in ids:
            raise ProtocolError(f"duplicate agent id {agent.id}")
        ids.add(agent.id)
        for label, cell in (("start", agent.start), ("goal", agent.goal)):
            if cell[0] >= width or cell[1] >= height:
                raise ProtocolError(f"agent {agent.id} {label} is outside the world")
            if cell in blocked:
                raise ProtocolError(f"agent {agent.id} {label} is blocked")
        agents.append(agent)

    metadata = _json_object(obj.get("metadata", {}), "problem.metadata")
    return Problem(
        _string(obj["problem_id"], "problem.problem_id", nonempty=True),
        objective,
        width,
        height,
        blocked,
        tuple(agents),
        metadata,
    )


def _parse_options(value: Any) -> SolveOptions:
    obj = _object(value, "options")
    fields = {"time_limit_ms", "random_seed", "node_limit"}
    _closed(obj, fields, "options")
    _required(obj, fields, "options")
    return SolveOptions(
        _integer(obj["time_limit_ms"], "options.time_limit_ms"),
        _integer(obj["random_seed"], "options.random_seed"),
        _integer(obj["node_limit"], "options.node_limit"),
    )


@dataclass(frozen=True)
class SolverMetadata:
    name: str
    version: str
    commit: str = ""
    configuration: Mapping[str, Any] = field(default_factory=dict)

    def to_dict(self) -> JsonObject:
        result: JsonObject = {
            "name": _string(self.name, "solver.name", nonempty=True),
            "version": _string(self.version, "solver.version", nonempty=True),
        }
        if self.commit:
            result["commit"] = _string(self.commit, "solver.commit")
        if self.configuration:
            result["configuration"] = _json_object(
                self.configuration, "solver.configuration"
            )
        return result


@dataclass(frozen=True)
class AgentPath:
    agent_id: int
    cells: Tuple[Coordinate, ...]

    @classmethod
    def from_cells(cls, agent_id: int, cells: Sequence[Sequence[int]]) -> "AgentPath":
        return cls(
            _integer(agent_id, "agent path id", 1),
            tuple(_coordinate(cell, "agent path cell") for cell in cells),
        )


@dataclass(frozen=True)
class RawSolveResult:
    status: str
    paths: Tuple[AgentPath, ...] = ()
    statistics: Mapping[str, Any] = field(default_factory=dict)
    message: str = ""
    problem_id: str = ""
    solver: SolverMetadata | None = None

    @classmethod
    def success(
        cls,
        paths: Sequence[AgentPath],
        *,
        statistics: Mapping[str, Any] | None = None,
        message: str = "",
    ) -> "RawSolveResult":
        return cls("success", tuple(paths), statistics or {}, message)

    @classmethod
    def infeasible(cls, message: str = "", **kwargs: Any) -> "RawSolveResult":
        return cls("infeasible", statistics=kwargs.get("statistics", {}), message=message)

    @classmethod
    def timeout(cls, message: str = "", **kwargs: Any) -> "RawSolveResult":
        return cls("timeout", statistics=kwargs.get("statistics", {}), message=message)

    @classmethod
    def unsupported(cls, message: str = "", **kwargs: Any) -> "RawSolveResult":
        return cls("unsupported", statistics=kwargs.get("statistics", {}), message=message)

    @classmethod
    def error(cls, message: str = "", **kwargs: Any) -> "RawSolveResult":
        return cls("error", statistics=kwargs.get("statistics", {}), message=message)

    def bound_to(self, problem_id: str, solver: SolverMetadata) -> "RawSolveResult":
        return RawSolveResult(
            self.status, self.paths, self.statistics, self.message, problem_id, solver
        )

    def to_dict(self) -> JsonObject:
        if self.status not in {"success", "infeasible", "timeout", "unsupported", "error"}:
            raise ProtocolError(f"invalid result status: {self.status}")
        if not self.problem_id:
            raise ProtocolError("result has not been bound to a problem")
        if self.solver is None or not self.solver.name or not self.solver.version:
            raise ProtocolError("result has no valid solver metadata")
        if self.status == "success" and not self.paths:
            raise ProtocolError("a success result must contain paths")
        if self.status != "success" and self.paths:
            raise ProtocolError("a non-success result must not contain paths")
        path_ids = [path.agent_id for path in self.paths]
        if len(set(path_ids)) != len(path_ids):
            raise ProtocolError("result contains duplicate agent paths")
        encoded_paths = []
        for path in self.paths:
            _integer(path.agent_id, "path.agent_id", 1)
            if not path.cells:
                raise ProtocolError("a success path must contain at least one cell")
            cells = [
                _coordinate(cell, f"path[{path.agent_id}].cells[{index}]")
                for index, cell in enumerate(path.cells)
            ]
            encoded_paths.append({
                "agent_id": path.agent_id,
                "cells": [[cell[0], cell[1]] for cell in cells],
            })
        result: JsonObject = {
            "protocol_version": RAW_SOLVE_RESULT_PROTOCOL_V1,
            "problem_id": self.problem_id,
            "solver": self.solver.to_dict(),
            "status": self.status,
            "paths": encoded_paths,
        }
        if self.statistics:
            result["statistics"] = _json_object(self.statistics, "result.statistics")
        if self.message:
            result["message"] = _string(self.message, "result.message")
        return result

    def to_json(self, *, pretty: bool = False) -> str:
        if pretty:
            return json.dumps(self.to_dict(), indent=2, ensure_ascii=False, allow_nan=False)
        return json.dumps(
            self.to_dict(), separators=(",", ":"), ensure_ascii=False, allow_nan=False
        )


def read_request(stream: TextIO = sys.stdin) -> SolveRequest:
    return SolveRequest.from_stream(stream)


def write_result(result: RawSolveResult, stream: TextIO = sys.stdout) -> None:
    stream.write(result.to_json())
    stream.write("\n")
    stream.flush()
