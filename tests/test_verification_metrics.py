import json
from pathlib import Path
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import Mock, patch

from scripts.verify_with_metrics import (Recorder, compare, digest_files, discover_verification_files,
                                        read_samples, run_with_helper, summarize,
                                        validate_publication_report, validate_selection, write_report)


def case(name="example_00", elapsed=0.1, status="AC"):
    return {"testcase": {"name": name}, "elapsed": elapsed, "status": status, "exitcode": 0, "output": "123\n"}


class MetricsTests(unittest.TestCase):
    def selection(self, selected=None):
        return {"schema_version": 1, "mode": "affected", "base_sha": "base",
                "head_sha": "head", "reason": "dependency closure", "repeats": 1,
                "all_verify": ["verify/a.test.cpp", "verify/b.test.cpp"],
                "selected_verify": ["verify/a.test.cpp"] if selected is None else selected}

    def selected_report(self, selected=None):
        plan = self.selection(selected)
        return {"schema": 1, "succeeded": True, "revision": "head", "selection": plan,
                "results": [
                    ({"path": path, "environment_index": 0, "status": "passed",
                      "repeat_count": 1, "runs": [[{"elapsed": 0.2}]],
                      "compile_seconds": 0.1, "solution_seconds": 0.2}
                     if path in plan["selected_verify"] else
                     {"path": path, "environment_index": 0, "status": "not_selected"})
                    for path in plan["all_verify"]]}

    def test_explicit_empty_selection_never_runs_all(self):
        plan = self.selection([])
        report = {"results": []}
        # No helper dependencies/output/path arguments are needed for an empty plan.
        self.assertTrue(run_with_helper(SimpleNamespace(selection=plan), report))
        self.assertEqual(len(report["results"]), 2)
        self.assertTrue(all(r["status"] == "not_selected" for r in report["results"]))
        self.assertTrue(all("solution_seconds" not in r for r in report["results"]))

    def test_selection_inventory_and_revision_are_checked(self):
        plan = self.selection()
        self.assertEqual(validate_selection(plan, plan["all_verify"], "head"), {"verify/a.test.cpp"})
        for changes in ({"head_sha": "stale"}, {"all_verify": []},
                        {"selected_verify": ["verify/missing.test.cpp"]},
                        {"selected_verify": ["verify/a.test.cpp"] * 2},
                        {"mode": "full"}, {"reason": ""}):
            with self.subTest(changes=changes), self.assertRaises(ValueError):
                validate_selection(dict(plan, **changes), plan["all_verify"], "head")

    def test_partial_and_documentation_only_reports_publish_honestly(self):
        for selected in ([], ["verify/a.test.cpp"]):
            report = self.selected_report(selected)
            self.assertEqual(validate_publication_report(report, self.selection()["all_verify"], "head"), set(selected))
        skip = {"status": "not_selected"}
        self.assertEqual(compare(skip, {"status": "passed", "solution_seconds": 1}, {}, {})[0], None)
        self.assertIn("今回未実行", compare(skip, None, {}, None)[1])

    def test_missing_failed_and_unmeasured_selected_rows_block_publication(self):
        for status in ("failed", "not_run", "ignored", "not_selected"):
            report = self.selected_report()
            report["results"][0]["status"] = status
            with self.subTest(status=status), self.assertRaises(ValueError):
                validate_publication_report(report, self.selection()["all_verify"])
        for field in ("runs", "compile_seconds", "solution_seconds"):
            report = self.selected_report()
            del report["results"][0][field]
            with self.subTest(field=field), self.assertRaises(ValueError):
                validate_publication_report(report, self.selection()["all_verify"])
        report = self.selected_report()
        report["results"].pop(0)
        with self.assertRaises(ValueError):
            validate_publication_report(report, self.selection()["all_verify"])

    def test_stale_unselected_measurements_and_unexplained_skips_are_rejected(self):
        for field, value in (("solution_seconds", 0), ("compile_seconds", 0.1),
                             ("runs", []), ("repeat_count", 1), ("delta_percent", -20),
                             ("verify_wall_seconds", 1.5), ("compiler", "old compiler")):
            report = self.selected_report()
            report["results"][1][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                validate_publication_report(report, self.selection()["all_verify"])
        report = self.selected_report()
        del report["selection"]
        with self.assertRaises(ValueError):
            validate_publication_report(report, self.selection()["all_verify"])
        report = self.selected_report()
        with self.assertRaises(ValueError):
            validate_publication_report(report, self.selection()["all_verify"], "new-head")

    def test_legacy_complete_reports_remain_valid(self):
        expected = self.selection()["all_verify"]
        report = self.selected_report(expected)
        del report["selection"]
        self.assertEqual(validate_publication_report(report, expected, "head"), set(expected))

    def test_docs_only_plan_runs_cli_without_verification_dependencies(self):
        from scripts.plan_verification import build_plan, validate_plan
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            def git(*args):
                return subprocess.check_output(["git", "-C", str(root), *args], text=True).strip()
            git("init", "-q")
            (root / "verify").mkdir()
            (root / "verify/a.test.cpp").write_text("// This must never be compiled in a docs-only run.\n")
            (root / "README.md").write_text("before\n")
            git("add", ".")
            git("-c", "user.name=Test", "-c", "user.email=test@example.invalid", "commit", "-qm", "base")
            base = git("rev-parse", "HEAD")
            (root / "README.md").write_text("after\n")
            git("add", ".")
            git("-c", "user.name=Test", "-c", "user.email=test@example.invalid", "commit", "-qm", "docs")
            plan = build_plan(root, base, "HEAD")
            self.assertEqual(plan["selected_verify"], [])
            validate_plan(plan, root)
            plan_path = root / "plan.json"
            plan_path.write_text(json.dumps(plan))
            runner = Path(__file__).resolve().parents[1] / "scripts/verify_with_metrics.py"
            subprocess.run([sys.executable, str(runner), "--plan", str(plan_path), "--repeats", "1"],
                           cwd=root, check=True, capture_output=True, text=True)
            report = json.loads((root / ".verification/current.json").read_text())
            self.assertEqual(report["results"][0]["status"], "not_selected")
            self.assertEqual(validate_publication_report(report, plan["all_verify"], plan["head_sha"]), set())
            self.assertFalse(list((root / ".verification").glob("run-*")))

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
