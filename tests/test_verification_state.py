"""Saved AC is reusable only for the exact source contract it actually verified."""
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from scripts import verification_state as state
from scripts.plan_verification import build_cached_plan, validate_plan


class VerificationStateTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.git("init", "-q")
        self.git("config", "user.email", "fixture@example.test")
        self.git("config", "user.name", "Fixture")
        self.git("config", "core.autocrlf", "false")
        self.write(".github/workflows/verify.yml", "repository: atcoder/ac-library\nref: " + "a" * 40 + "\n")
        self.write("requirements-dev.txt", "verifier==1\n")
        self.write(".verify-helper/config.toml", 'CXX = "g++"\n')
        self.write("blueberry/left.hpp", '#include "blueberry/leaf.hpp"\n')
        self.write("blueberry/leaf.hpp", "constexpr int value = 1;\n")
        self.write("blueberry/right.hpp", "constexpr int other = 2;\n")
        self.left = "verify/left.test.cpp"
        self.right = "verify/right.test.cpp"
        self.write(self.left, '#include "blueberry/left.hpp"\nint main() {}\n')
        self.write(self.right, '#include "blueberry/right.hpp"\nint main() {}\n')
        self.base = self.commit()
        self.saved = state.update_state(state.empty_state(), self.report(), self.root,
                                        "https://github.com/example/repo/actions/runs/1")

    def git(self, *args):
        return subprocess.run(["git", "-C", str(self.root), *args], check=True,
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE).stdout.decode().strip()

    def write(self, path, text):
        target = self.root / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(text, encoding="utf-8")

    def commit(self):
        self.git("add", ".")
        self.git("commit", "-qm", "fixture")
        return self.git("rev-parse", "HEAD")

    def report(self, day=1, status="passed", paths=None):
        rows = []
        for path in paths or (self.left, self.right):
            rows.append({"path": path, "status": status, "environment_index": 0,
                         "compiler": "fixture compiler", "flags": ["-O2"],
                         "verifier_sha256": hashlib.sha256((self.root / path).read_bytes()).hexdigest(),
                         "dataset_sha256": "d" * 64, "repeat_count": 1, "case_count": 1,
                         "compile_seconds": 0.2, "solution_seconds": 0.1,
                         "worst_case_seconds": 0.1, "min_solution_seconds": 0.1,
                         "max_solution_seconds": 0.1, "runs": [{"log": "original.json"}]})
        return {"schema": 1, "revision": self.git("rev-parse", "HEAD"),
                "generated_at": f"2026-10-{day:02}T00:00:00+00:00", "results": rows,
                "succeeded": status == "passed"}

    def selected(self, saved=None, force_full=False):
        plan = build_cached_plan(self.root, self.base, "HEAD", saved or self.saved, force_full=force_full)
        plan["repeats"] = 3 if force_full else 1
        return plan

    def test_docs_only_reuses_ac_without_rewriting_provenance(self):
        original = deepcopy(self.saved)
        self.write("README.md", "Documentation only.\n")
        self.commit()
        self.assertEqual(set(state.valid_successes(self.saved, self.root)), {self.left, self.right})
        self.assertEqual(self.selected()["selected_verify"], [])
        self.assertEqual(self.saved, original)
        self.assertEqual(self.saved["records"][self.left]["verified_revision"], self.base)
        self.assertEqual(self.saved["records"][self.left]["verified_at"], "2026-10-01T00:00:00+00:00")

    def test_transitive_header_change_selects_only_its_consumer(self):
        self.write("blueberry/leaf.hpp", "constexpr int value = 9;\n")
        self.commit()
        self.assertEqual(self.selected()["selected_verify"], [self.left])
        self.assertEqual(set(state.valid_successes(self.saved, self.root)), {self.right})

    def test_dirty_transitive_header_is_not_displayed_as_verified(self):
        self.write("blueberry/leaf.hpp", "constexpr int value = 9;\n")
        self.assertEqual(set(state.valid_successes(self.saved, self.root)), {self.right})

    def test_dirty_compiled_header_cannot_create_a_clean_source_receipt(self):
        original = (self.root / "blueberry/leaf.hpp").read_text()
        self.write("blueberry/leaf.hpp", "constexpr int value = 99;\n")
        saved = state.empty_state()
        with self.assertRaisesRegex(ValueError, "uncommitted source"):
            state.update_state(saved, self.report(paths=[self.left]), self.root)
        self.write("blueberry/leaf.hpp", original)
        self.assertEqual(state.valid_successes(saved, self.root), {})

    def test_non_acl_workflow_edit_does_not_invalidate_programs(self):
        path = ".github/workflows/verify.yml"
        self.write(path, (self.root / path).read_text() + "# documentation step\n")
        self.commit()
        self.assertEqual(self.selected()["selected_verify"], [])

    def test_execution_core_changes_invalidate_but_report_plumbing_does_not(self):
        path = "scripts/verify_with_metrics.py"
        core = ("def read_samples(payload, names):\n    return payload\n"
                "def digest_files(paths, root):\n    return paths\n"
                "class Recorder:\n    def execute(self, command):\n        return command\n"
                "def run_with_helper(args, report):\n    return True\n")
        self.write(path, core + "def write_report():\n    return 1\ndef main():\n    return 1\n")
        self.base = self.commit()
        saved = state.update_state(state.empty_state(), self.report(), self.root)
        self.write(path, "# Harmless commentary\n" + core +
                   "def write_report():\n    return 2\ndef main():\n    return 2\n")
        self.commit()
        self.assertEqual(self.selected(saved)["selected_verify"], [])
        self.write(path, core.replace("return True", "return False"))
        self.commit()
        self.assertEqual(self.selected(saved)["selected_verify"], [self.left, self.right])

    def test_present_invalid_execution_ast_is_not_reusable(self):
        self.write("scripts/verify_with_metrics.py", "def broken(\n")
        self.commit()
        self.assertEqual(state.fingerprints(self.root), {})
        self.assertEqual(state.valid_successes(self.saved, self.root), {})

    def test_execution_dump_is_stable_with_python312_keyword_interface(self):
        source = ("def read_samples(a,b): pass\ndef digest_files(a,b): pass\n"
                  "class Recorder: pass\ndef run_with_helper(a,b): pass\n")
        expected = state.execution_contract(source)
        original = state.ast.dump

        def legacy_dump(node, include_attributes=False):
            try:
                return original(node, include_attributes=include_attributes, show_empty=True)
            except TypeError:
                return original(node, include_attributes=include_attributes)

        with patch.object(state.ast, "dump", legacy_dump):
            self.assertEqual(state.execution_contract(source), expected)

    def test_contract_changes_each_select_all(self):
        for path, replacement in (
            ("requirements-dev.txt", "verifier==2\n"),
            (".verify-helper/config.toml", 'CXX = "clang++"\n'),
            (".github/workflows/verify.yml", "repository: atcoder/ac-library\nref: " + "b" * 40 + "\n"),
        ):
            with self.subTest(path=path):
                saved = state.update_state(state.empty_state(), self.report(), self.root)
                self.write(path, replacement)
                self.commit()
                self.assertEqual(self.selected(saved)["selected_verify"], [self.left, self.right])

    def test_new_driver_runs_and_deleted_driver_is_pruned(self):
        self.write("verify/new.test.cpp", "int main() {}\n")
        (self.root / self.left).unlink()
        self.commit()
        self.assertEqual(self.selected()["selected_verify"], ["verify/new.test.cpp"])
        empty_report = self.report(paths=[self.right])
        empty_report["results"] = []
        updated = state.update_state(self.saved, empty_report, self.root)
        self.assertNotIn(self.left, updated["records"])

    def test_unknown_dependency_invalidates_only_its_consumers(self):
        self.write("blueberry/leaf.hpp", '#include SOME_UNKNOWN_MACRO\n')
        self.commit()
        self.assertEqual(set(state.fingerprints(self.root)), {self.right})
        self.assertEqual(self.selected()["selected_verify"], [self.left])

    def test_failure_tombstone_prevents_seed_resurrection(self):
        failed = state.update_state(self.saved, self.report(day=2, status="failed", paths=[self.left]), self.root)
        self.assertNotIn(self.left, failed["records"])
        self.assertIn(self.left, failed["failures"])
        for merged in (state.merge_states(self.saved, failed), state.merge_states(failed, self.saved)):
            self.assertNotIn(self.left, state.valid_successes(merged, self.root))
        recovered = state.update_state(failed, self.report(day=3, paths=[self.left]), self.root)
        self.assertIn(self.left, recovered["records"])
        self.assertNotIn(self.left, recovered["failures"])

    def test_failure_at_unresolved_source_still_has_tombstone(self):
        self.write("blueberry/leaf.hpp", '#include UNKNOWN\n')
        self.commit()
        failed = state.update_state(self.saved, self.report(day=2, status="failed", paths=[self.left]), self.root)
        self.assertIn(self.left, failed["failures"])
        self.assertNotIn(self.left, failed["records"])

    def test_failure_wins_equal_timestamp_and_stale_pass_does_not_override(self):
        failed = state.update_state(self.saved, self.report(status="failed", paths=[self.left]), self.root)
        restored = state.update_state(failed, self.report(paths=[self.left]), self.root)
        self.assertIn(self.left, restored["failures"])
        self.assertNotIn(self.left, restored["records"])

    def test_overlapping_runs_are_ordered_by_completion_not_start(self):
        passed = self.report(day=3, paths=[self.left])
        passed["completed_at"] = "2026-10-04T00:00:00+00:00"
        saved = state.update_state(self.saved, passed, self.root)
        failed = self.report(day=2, status="failed", paths=[self.left])
        failed["completed_at"] = "2026-10-05T00:00:00+00:00"
        result = state.update_state(saved, failed, self.root)
        self.assertNotIn(self.left, result["records"])
        self.assertEqual(result["failures"][self.left]["verified_at"], failed["completed_at"])
        # Replaying the earlier-finished success must not restore its AC.
        replayed = state.update_state(result, passed, self.root)
        self.assertNotIn(self.left, replayed["records"])

    def test_corrupted_state_and_missing_measurements_fail_closed(self):
        corrupt = deepcopy(self.saved)
        corrupt["records"][self.left]["fingerprint"] = "broken"
        self.assertEqual(state.valid_successes(corrupt, self.root), {})
        path = self.root / "state.json"
        for text in ("{", "[]", json.dumps(corrupt)):
            path.write_text(text)
            self.assertEqual(state.load_state(path), state.empty_state())
        report = self.report()
        del report["results"][0]["runs"]
        with self.assertRaises(ValueError):
            state.update_state(self.saved, report, self.root)

    def test_all_environments_must_pass_and_driver_digest_must_match(self):
        report = self.report(paths=[self.left])
        report["results"][0]["verifier_sha256"] = "0" * 64
        with self.assertRaises(ValueError):
            state.update_state(state.empty_state(), report, self.root)
        report = self.report(day=2, paths=[self.left])
        other = deepcopy(report["results"][0])
        other.update(environment_index=1, status="failed")
        report["results"].append(other)
        result = state.update_state(self.saved, report, self.root)
        self.assertNotIn(self.left, result["records"])

    def test_unselected_rows_never_create_new_ac_and_local_url_can_be_none(self):
        report = self.report(status="not_selected")
        self.assertEqual(state.update_state(state.empty_state(), report, self.root, run_url=None), state.empty_state())

    def test_manual_full_still_selects_every_target_and_three_repeats(self):
        plan = self.selected(force_full=True)
        self.assertEqual((plan["mode"], plan["repeats"], plan["selected_verify"]),
                         ("full", 3, [self.left, self.right]))

    def test_143_saved_receipts_select_zero_for_a_documentation_change(self):
        paths = [self.left, self.right]
        for index in range(141):
            path = f"verify/additional-{index:03}.test.cpp"
            self.write(path, "int main() {}\n")
            paths.append(path)
        self.base = self.commit()
        saved = state.update_state(state.empty_state(), self.report(paths=paths), self.root)
        self.write("README.md", "Only documentation changes.\n")
        self.commit()
        plan = self.selected(saved)
        self.assertEqual(len(saved["records"]), 143)
        self.assertEqual((plan["mode"], plan["selected_verify"], plan["compile_required"]),
                         ("affected", [], False))

    def test_changed_selection_state_is_rejected(self):
        plan = self.selected()
        self.write(".verification/previous-state.json", json.dumps(self.saved))
        validate_plan(plan, self.root)
        self.write(".verification/previous-state.json", json.dumps(state.empty_state()))
        with self.assertRaisesRegex(ValueError, "saved verification state"):
            validate_plan(plan, self.root)


if __name__ == "__main__":
    unittest.main()
