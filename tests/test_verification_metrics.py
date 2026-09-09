import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import Mock, patch

from scripts.verify_with_metrics import Recorder, compare, digest_files, discover_verification_files, read_samples, summarize, write_report


def case(name="example_00", elapsed=0.1, status="AC"):
    return {"testcase": {"name": name}, "elapsed": elapsed, "status": status, "exitcode": 0, "output": "123\n"}


class MetricsTests(unittest.TestCase):
    def test_discovery_ignores_generated_copies(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            actual = root / "verify/graph/example.test.cpp"
            generated = root / "_site/verify/verify/graph/example.test.cpp"
            for path in (actual, generated):
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text("// fixture\n")
            self.assertEqual(discover_verification_files(root / "verify"), [actual])

    def test_structured_log(self):
        samples = read_samples([case()], ["example_00"])
        self.assertEqual(samples[0]["elapsed"], 0.1)
        self.assertNotIn("output", samples[0])

    def test_missing_duplicate_and_invalid_times(self):
        payloads = [[], [case(), case()], [case("other")], [case(elapsed=None)],
                    [case(elapsed=-1)], [case(elapsed=float("nan"))], [case(elapsed=float("inf"))]]
        for payload in payloads:
            with self.subTest(payload=payload), self.assertRaises(ValueError):
                read_samples(payload, ["example_00"])

    def test_median_is_total_of_cases_not_median_case(self):
        runs = [[{"elapsed": a}, {"elapsed": b}] for a, b in [(1, 2), (2, 5), (20, 30)]]
        result = summarize(runs)
        self.assertEqual(result["solution_seconds"], 7)
        self.assertEqual(result["worst_case_seconds"], 5)
        self.assertEqual(result["min_solution_seconds"], 3)
        self.assertEqual(result["max_solution_seconds"], 50)

    def test_comparisons_and_initial_run(self):
        row = {"status": "passed", "solution_seconds": 8, "compiler": "g++",
               "flags": ["-O2"], "dataset_sha256": "a", "verifier_sha256": "b", "repeat_count": 3}
        old = dict(row, solution_seconds=10)
        environment = {"cpu": "test-cpu"}
        baseline = {"schema": 1, "environment": environment}
        self.assertAlmostEqual(compare(row, old, environment, baseline)[0], -20)
        self.assertIsNone(compare(row, None, environment, None)[0])
        for key in ("compiler", "flags", "dataset_sha256", "verifier_sha256", "repeat_count"):
            with self.subTest(key=key):
                changed = dict(row, **{key: "changed"})
                self.assertIsNone(compare(changed, old, environment, baseline)[0])
        self.assertIsNone(compare(row, old, {"cpu": "different"}, baseline)[0])
        self.assertIsNone(compare(dict(row, status="failed"), old, environment, baseline)[0])
        self.assertIsNone(compare(row, dict(old, solution_seconds=0), environment, baseline)[0])

    def test_dataset_changes_invalidate_hash(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            data = root / "example.in"
            data.write_text("1\n")
            first = digest_files([data], root)
            data.write_text("2\n")
            self.assertNotEqual(first, digest_files([data], root))

    def test_runner_repeats_serial_and_preserves_checker(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "example_00.in").write_text("1\n")
            row = {"path": "verify/test.cpp", "environment_index": 0, "status": "not_run", "runs": []}
            recorder = Recorder({"results": [row]}, root, 3)
            recorder.active = row
            command = ["oj", "test", "-d", str(root), "-c", "a.out", "--judge-command", "checker"]

            def fake_run(cmd, **kwargs):
                self.assertIn("--judge-command", cmd)
                self.assertNotIn("-j", cmd)
                Path(cmd[-1]).write_text(json.dumps([case()]))
                return subprocess.CompletedProcess(cmd, 0)

            with patch("scripts.verify_with_metrics.subprocess.run", side_effect=fake_run) as run:
                recorder.execute(Mock(), command)
            self.assertEqual(run.call_count, 3)
            self.assertEqual(row["status"], "passed")
            self.assertEqual(row["case_count"], 1)
            self.assertEqual(len(row["runs"]), 3)

    def test_failed_and_missing_logs_never_become_success(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "example_00.in").write_text("1\n")
            for missing in (False, True):
                row = {"path": "verify/test.cpp", "environment_index": 0, "status": "not_run", "runs": []}
                recorder = Recorder({"results": [row]}, root, 3)
                recorder.active = row

                def fake_run(cmd, **kwargs):
                    if not missing:
                        Path(cmd[-1]).write_text(json.dumps([case(status="WA")]))
                    return subprocess.CompletedProcess(cmd, 1)

                with patch("scripts.verify_with_metrics.subprocess.run", side_effect=fake_run) as run:
                    with self.assertRaises(RuntimeError):
                        recorder.execute(Mock(), ["oj", "test", "-d", str(root)])
                self.assertEqual(run.call_count, 1)
                self.assertNotIn("solution_seconds", row)

    def test_failed_report_is_written(self):
        with tempfile.TemporaryDirectory() as temporary:
            report = {"revision": "test", "generated_at": "now", "environment": {"cpu": "test"},
                      "results": [{"path": "verify/test.cpp", "environment_index": 0, "status": "failed"}]}
            with patch.dict("os.environ", {}, clear=True):
                write_report(report, None, Path(temporary))
            saved = json.loads((Path(temporary) / "current.json").read_text())
            self.assertIsNone(saved["results"][0]["delta_percent"])
            self.assertNotIn("solution_seconds", saved["results"][0])


if __name__ == "__main__":
    unittest.main()
