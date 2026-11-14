import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from scripts.recheck_verify_performance import choose_targets, comparable, main, recheck


class RecheckTests(unittest.TestCase):
    def fixture(self, root):
        (root / "verify").mkdir()
        (root / "logs").mkdir()
        rows = []
        for name, worst in (("a", 8.0), ("b", 6.0), ("c", 5.1), ("d", 3.0)):
            source = f'#define PROBLEM "https://judge.yosupo.jp/problem/{name}"\n'.encode()
            (root / "verify" / f"{name}.test.cpp").write_bytes(source)
            (root / "logs" / f"{name}.json").write_text(json.dumps(
                [{"name": "case", "status": "AC", "exitcode": 0, "elapsed": worst}]), encoding="utf-8")
            rows.append({"path": f"verify/{name}.test.cpp", "environment_index": 0,
                         "status": "passed", "compiler": "g++ test", "flags": ["-O2"],
                         "dataset_sha256": "dataset", "verifier_sha256": hashlib.sha256(source).hexdigest(),
                         "repeat_count": 1, "case_count": 1,
                         "runs": [{"log": f"logs/{name}.json", "solution_seconds": worst, "worst_case_seconds": worst}],
                         "worst_case_seconds": worst})
        return {"schema": 1, "succeeded": True, "revision": "revision",
                "environment": {"cpu": "test", "tle_seconds": 60}, "results": rows}

    def limits(self):
        return {name: {"seconds": 10, "path": f"data/{name}/info.toml", "sha256": "limit"}
                for name in ("a", "b", "c", "d")}

    def test_top_two_are_rechecked_with_three_runs_and_matching_conditions(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            initial = self.fixture(root)
            commands = []
            def executor(command, cwd, log, timeout):
                commands.append(command)
                log.write_text("completed", encoding="utf-8")
                case_log = log.parent / "case.json"
                case_log.write_text(json.dumps([{"name": "case", "status": "AC", "exitcode": 0,
                                                "elapsed": 4.0}]), encoding="utf-8")
                row = next(row for row in initial["results"] if row["path"] == command[-1]).copy()
                row.update(repeat_count=3, runs=[{"log": str(case_log), "solution_seconds": 4.0,
                                                "worst_case_seconds": 4.0}] * 3, worst_case_seconds=4.0)
                report = dict(initial, results=[row])
                (log.parent / "current.json").write_text(json.dumps(report), encoding="utf-8")
                return {"exit_code": 0, "timed_out": False}
            with patch("scripts.recheck_verify_performance.git_revision", return_value=("revision", False)):
                result = recheck(initial, root, self.limits(), root / "out", executor=executor)
            self.assertEqual([row["path"] for row in result["rechecks"]], ["verify/a.test.cpp", "verify/b.test.cpp"])
            self.assertTrue(all(cmd[cmd.index("--repeats") + 1] == "3" for cmd in commands))
            self.assertTrue(all(row["status"] == "measured" for row in result["rechecks"]))
            self.assertNotIn("repeated_to_initial", result["rechecks"][0])
            self.assertTrue((root / "out/recheck-1/execution.log").is_file())

    def test_unmatched_or_incomplete_conditions_are_not_compared(self):
        initial = {"revision": "rev", "environment": {"cpu": "one"}}
        row = {"compiler": "gcc", "flags": ["-O2"], "dataset_sha256": "input",
               "verifier_sha256": "driver", "worst_case_seconds": 3, "status": "passed",
               "repeat_count": 1, "case_count": 1,
               "runs": [{"log": "case.json", "solution_seconds": 3, "worst_case_seconds": 3}]}
        repeat = dict(row, repeat_count=3, runs=row["runs"] * 3)
        self.assertTrue(comparable(initial, initial, row, repeat))
        for key in ("compiler", "flags", "dataset_sha256", "verifier_sha256", "runs"):
            with self.subTest(key=key):
                self.assertFalse(comparable(initial, initial, row, dict(repeat, **{key: None})))
        self.assertFalse(comparable(initial, dict(initial, environment={"cpu": "other"}), row, repeat))
        self.assertFalse(comparable(initial, dict(initial, revision="different"), row, repeat))

    def test_failures_and_timeouts_are_kept_without_times(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            initial = self.fixture(root)
            responses = iter([{"exit_code": 1, "timed_out": False}, {"exit_code": None, "timed_out": True}])
            def executor(command, cwd, log, timeout):
                log.write_text("partial output", encoding="utf-8")
                return next(responses)
            with patch("scripts.recheck_verify_performance.git_revision", return_value=("revision", False)):
                result = recheck(initial, root, self.limits(), root / "out", executor=executor)
            self.assertEqual([row["status"] for row in result["rechecks"]], ["failed", "timeout"])
            self.assertTrue(all("repeated_to_initial" not in row for row in result["rechecks"]))
            self.assertEqual((root / "out/recheck-2/execution.log").read_text(), "partial output")

    def test_stale_revision_is_not_rechecked(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            initial = self.fixture(root)
            def never(*args):
                self.fail("recheck should not execute")
            with patch("scripts.recheck_verify_performance.git_revision", return_value=("stale", False)):
                result = recheck(initial, root, self.limits(), root / "out", executor=never)
            self.assertTrue(all(row["status"] == "unmeasured" for row in result["rechecks"]))

    def test_unrelated_failed_initial_case_does_not_block_completed_cases(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            initial = self.fixture(root)
            initial["succeeded"] = False
            initial["results"][-1]["status"] = "failed"
            commands = []
            def executor(command, cwd, log, timeout):
                commands.append(command)
                log.write_text("failed", encoding="utf-8")
                return {"exit_code": 1, "timed_out": False}
            with patch("scripts.recheck_verify_performance.git_revision", return_value=("revision", False)):
                result = recheck(initial, root, self.limits(), root / "out", executor=executor)
            self.assertEqual(len(commands), 2)
            self.assertEqual(result["screening_counts"]["unmeasured"], 1)
            self.assertEqual([row["status"] for row in result["rechecks"]], ["failed", "failed"])

    def test_detects_wa_and_preserves_log(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            initial = self.fixture(root)
            def executor(command, cwd, log, timeout):
                log.write_text("wrong answer", encoding="utf-8")
                case_log = log.parent / "case.json"
                case_log.write_text(json.dumps([{"name": "case", "status": "WA", "exitcode": 0}]), encoding="utf-8")
                report = {"results": [{"runs": [{"log": str(case_log)}]}]}
                (log.parent / "current.json").write_text(json.dumps(report), encoding="utf-8")
                return {"exit_code": 1, "timed_out": False}
            with patch("scripts.recheck_verify_performance.git_revision", return_value=("revision", False)):
                result = recheck(initial, root, self.limits(), root / "out", executor=executor)
            self.assertTrue(all(row["status"] == "incorrect" for row in result["rechecks"]))
            self.assertTrue((root / "out/recheck-1/case.json").exists())
            input_file = root / "input.json"
            input_file.write_text(json.dumps(initial), encoding="utf-8")
            with patch("scripts.recheck_verify_performance.official_limits", return_value=self.limits()), \
                 patch("scripts.recheck_verify_performance.recheck", return_value=result), \
                 patch("scripts.recheck_verify_performance.git_revision", return_value=("revision", False)):
                self.assertEqual(main(["--report", str(input_file), "--problem-repo", str(root),
                                       "--output", str(root / "result")]), 1)

    def test_case_tle_is_recorded_separately_from_wrong_answer(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            initial = self.fixture(root)
            def executor(command, cwd, log, timeout):
                case_log = log.parent / "case.json"
                case_log.write_text(json.dumps([{"name": "case", "status": "TLE", "exitcode": -9}]), encoding="utf-8")
                (log.parent / "current.json").write_text(json.dumps(
                    {"results": [{"runs": [{"log": str(case_log)}]}]}), encoding="utf-8")
                return {"exit_code": 1, "timed_out": False}
            with patch("scripts.recheck_verify_performance.git_revision", return_value=("revision", False)):
                result = recheck(initial, root, self.limits(), root / "out", executor=executor)
            self.assertTrue(all(row["status"] == "timeout" for row in result["rechecks"]))

    def test_missing_or_changed_problem_checkout_is_not_used(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            report = root / "input.json"
            report.write_text(json.dumps({"schema": 1, "results": []}), encoding="utf-8")
            for actual in ((None, None), ("other", False), ("wanted", True)):
                with self.subTest(actual=actual), \
                     patch("scripts.recheck_verify_performance.git_revision", return_value=actual), \
                     patch("scripts.recheck_verify_performance.recheck") as run:
                    output = root / "result"
                    self.assertEqual(main(["--report", str(report), "--problem-repo", str(root),
                                           "--expected-problem-revision", "wanted", "--output", str(output)]), 0)
                    run.assert_not_called()
                    self.assertEqual(json.loads((output / "performance.json").read_text(encoding="utf-8"))
                                     ["screening_counts"], {"unavailable": 1})

    def test_recheck_on_different_environment_is_not_compared_to_original(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            initial = self.fixture(root)
            def executor(command, cwd, log, timeout):
                case_log = log.parent / "case.json"
                case_log.write_text(json.dumps([{"name": "case", "status": "AC", "exitcode": 0,
                                                "elapsed": 4}]), encoding="utf-8")
                row = next(row for row in initial["results"] if row["path"] == command[-1]).copy()
                row.update(repeat_count=3, runs=[{"log": str(case_log), "solution_seconds": 4,
                                                "worst_case_seconds": 4}] * 3, worst_case_seconds=4)
                report = dict(initial, environment={"cpu": "another machine"}, results=[row])
                (log.parent / "current.json").write_text(json.dumps(report), encoding="utf-8")
                return {"exit_code": 0, "timed_out": False}
            with patch("scripts.recheck_verify_performance.git_revision", return_value=("revision", False)):
                result = recheck(initial, root, self.limits(), root / "out", executor=executor)
            self.assertTrue(all(row["status"] == "different_conditions" for row in result["rechecks"]))
            self.assertTrue(all("repeated_to_initial" not in row for row in result["rechecks"]))
            self.assertEqual(result["rechecks"][0]["recheck_environment"]["cpu"], "another machine")
            self.assertIn("environment", result["rechecks"][0]["condition_mismatches"])

    def test_initial_log_is_resolved_from_downloaded_artifact(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            initial = self.fixture(root)
            downloaded = root / ".verification/run-initial"
            downloaded.mkdir(parents=True)
            for row in initial["results"]:
                original = root / row["runs"][0]["log"]
                destination = downloaded / original.name
                destination.write_bytes(original.read_bytes())
                row["runs"][0]["log"] = f"/old/runner/.verification/run-initial/{original.name}"
            def executor(command, cwd, log, timeout):
                return {"exit_code": 1, "timed_out": False}
            with patch("scripts.recheck_verify_performance.git_revision", return_value=("revision", False)):
                result = recheck(initial, root, self.limits(), root / "out", executor=executor)
            self.assertEqual(len(result["rechecks"]), 2)

    def test_missing_case_log_removes_candidate_instead_of_assuming_success(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            initial = self.fixture(root)
            (root / "logs/a.json").unlink()
            def executor(command, cwd, log, timeout):
                log.write_text("failed", encoding="utf-8")
                return {"exit_code": 1, "timed_out": False}
            with patch("scripts.recheck_verify_performance.git_revision", return_value=("revision", False)):
                result = recheck(initial, root, self.limits(), root / "out", executor=executor)
            self.assertEqual(result["screening_counts"]["unmeasured"], 1)
            self.assertEqual([row["path"] for row in result["rechecks"]], ["verify/b.test.cpp", "verify/c.test.cpp"])

    def test_real_helper_process_creates_three_case_logs(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            initial = self.fixture(root)
            (root / "scripts").mkdir()
            (root / "initial.json").write_text(json.dumps(initial), encoding="utf-8")
            (root / "scripts/verify_with_metrics.py").write_text('''
import json, pathlib, sys
root = pathlib.Path(__file__).resolve().parents[1]
report = json.loads((root / "initial.json").read_text(encoding="utf-8"))
output = pathlib.Path(sys.argv[sys.argv.index("--output") + 1])
target = sys.argv[-1]
row = next(row for row in report["results"] if row["path"] == target)
row["runs"] = []
for index in range(3):
    log = output / (str(index) + ".json")
    log.write_text(json.dumps([{"name": "case", "status": "AC", "exitcode": 0, "elapsed": 1}]))
    row["runs"].append({"log": str(log), "solution_seconds": 1, "worst_case_seconds": 1})
row.update(repeat_count=3, worst_case_seconds=1)
report["results"] = [row]
(output / "current.json").write_text(json.dumps(report))
print("fake verifier completed")
''', encoding="utf-8")
            with patch("scripts.recheck_verify_performance.git_revision", return_value=("revision", False)):
                result = recheck(initial, root, self.limits(), root / "out", wall_timeout=10)
            self.assertEqual([row["status"] for row in result["rechecks"]], ["measured", "measured"])
            self.assertEqual(len(list((root / "out/recheck-1").glob("[0-9].json"))), 3)
            self.assertIn("fake verifier completed", (root / "out/recheck-1/execution.log").read_text())

    def test_duplicate_environments_recheck_each_driver_once(self):
        rows = [{"path": "a", "environment_index": 1, "assessment": "review_headroom", "ratio": 0.8},
                {"path": "a", "environment_index": 0, "assessment": "review_headroom", "ratio": 0.7},
                {"path": "b", "environment_index": 0, "assessment": "review_headroom", "ratio": 0.6},
                {"path": "c", "environment_index": 0, "assessment": "unmeasured", "ratio": None}]
        self.assertEqual([row["path"] for row in choose_targets(rows)], ["a", "b"])


if __name__ == "__main__":
    unittest.main()
