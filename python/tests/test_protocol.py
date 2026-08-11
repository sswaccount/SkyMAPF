import json
import pathlib
import unittest

from skymapf import (
    AgentPath,
    ProtocolError,
    RawSolveResult,
    SolveRequest,
    SolverMetadata,
)


ROOT = pathlib.Path(__file__).resolve().parents[2]
VALID_REQUEST = ROOT / "test/data/protocol/v1/valid/solve-request.json"


class ProtocolTest(unittest.TestCase):
    def request_document(self):
        return json.loads(VALID_REQUEST.read_text(encoding="utf-8"))

    def test_reads_golden_request_and_exposes_problem_helpers(self):
        request = SolveRequest.from_dict(self.request_document())

        self.assertEqual(request.problem.problem_id, "case-7")
        self.assertEqual(len(request.problem.agents), 2)
        self.assertEqual(request.problem.agent_by_id(1).goal, (1, 0))
        self.assertEqual(
            request.problem.neighbors((0, 0)),
            ((0, 0), (1, 0), (0, 1)),
        )
        self.assertEqual(request.options.time_limit_ms, 250)

    def test_rejects_unknown_fields_and_bool_in_integer_field(self):
        document = self.request_document()
        document["extra"] = True
        with self.assertRaisesRegex(ProtocolError, "unknown properties"):
            SolveRequest.from_dict(document)

        document = self.request_document()
        document["options"]["node_limit"] = False
        with self.assertRaisesRegex(ProtocolError, "integer"):
            SolveRequest.from_dict(document)

    def test_rejects_semantically_invalid_problem(self):
        document = self.request_document()
        document["problem"]["world"]["blocked_cells"] = [[0, 0]]
        with self.assertRaisesRegex(ProtocolError, "start is blocked"):
            SolveRequest.from_dict(document)

    def test_encodes_canonical_raw_result_shape(self):
        result = RawSolveResult.success(
            [AgentPath.from_cells(1, [(0, 0), (1, 0)])],
            statistics={"expanded_nodes": 3},
        ).bound_to("case-7", SolverMetadata("demo", "1.0"))

        document = json.loads(result.to_json())
        self.assertEqual(document["protocol_version"], "skymapf.raw-solve-result.v1")
        self.assertEqual(document["paths"][0]["cells"], [[0, 0], [1, 0]])
        self.assertEqual(document["statistics"]["expanded_nodes"], 3)

    def test_rejects_status_path_inconsistency(self):
        result = RawSolveResult.success([]).bound_to(
            "case-7", SolverMetadata("demo", "1.0")
        )
        with self.assertRaisesRegex(ProtocolError, "must contain paths"):
            result.to_dict()


if __name__ == "__main__":
    unittest.main()
