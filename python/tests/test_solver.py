import io
import json
import pathlib
import unittest

from skymapf import AgentPath, RawSolveResult, UnsupportedProblem, solver


ROOT = pathlib.Path(__file__).resolve().parents[2]
VALID_REQUEST = ROOT / "test/data/protocol/v1/valid/solve-request.json"


def request_stream():
    return io.StringIO(VALID_REQUEST.read_text(encoding="utf-8"))


class SolverApplicationTest(unittest.TestCase):
    def test_decorator_runs_protocol_application(self):
        @solver("python_demo", "1.2", commit="abc")
        def solve(problem, options):
            self.assertEqual(options.random_seed, 42)
            return RawSolveResult.success([
                AgentPath.from_cells(agent.id, [agent.start, agent.goal])
                for agent in problem.agents
            ])

        stdout = io.StringIO()
        stderr = io.StringIO()
        self.assertEqual(solve.main(request_stream(), stdout, stderr), 0)
        document = json.loads(stdout.getvalue())
        self.assertEqual(document["problem_id"], "case-7")
        self.assertEqual(document["solver"]["name"], "python_demo")
        self.assertEqual(document["solver"]["commit"], "abc")
        self.assertEqual(document["status"], "success")
        self.assertEqual(stderr.getvalue(), "")

    def test_maps_unsupported_and_solver_exception(self):
        @solver("limited", "1.0")
        def unsupported(problem, options):
            raise UnsupportedProblem("only four-neighbor grids are supported")

        output = io.StringIO()
        self.assertEqual(unsupported.main(request_stream(), output, io.StringIO()), 0)
        self.assertEqual(json.loads(output.getvalue())["status"], "unsupported")

        @solver("broken", "1.0")
        def broken(problem, options):
            raise RuntimeError("boom")

        output = io.StringIO()
        errors = io.StringIO()
        self.assertEqual(broken.main(request_stream(), output, errors), 0)
        document = json.loads(output.getvalue())
        self.assertEqual(document["status"], "error")
        self.assertIn("RuntimeError: boom", document["message"])
        self.assertIn("Traceback", errors.getvalue())

    def test_invalid_request_is_process_failure_without_stdout(self):
        @solver("demo", "1.0")
        def solve(problem, options):
            return RawSolveResult.infeasible()

        stdout = io.StringIO()
        stderr = io.StringIO()
        self.assertEqual(solve.main(io.StringIO("not-json"), stdout, stderr), 2)
        self.assertEqual(stdout.getvalue(), "")
        self.assertIn("invalid SkyMAPF request", stderr.getvalue())


if __name__ == "__main__":
    unittest.main()
