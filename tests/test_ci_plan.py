import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
import yaml

from scripts.plan_verification import build_plan, event_base, includes, trusted_base, validate_plan, verified_ancestor
from scripts.run_ci_unit_tests import without_cpp


class CiPlanTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.git("init", "-q")
        self.git("config", "user.name", "CI plan test")
        self.git("config", "user.email", "ci-plan@example.invalid")
        self.write("blueberry/lib/leaf.hpp", "#pragma once\n")
        self.write("blueberry/lib/shared.hpp", '#include "leaf.hpp"\n')
        self.write("blueberry/lib/other.hpp", "#pragma once\n")
        self.write("verify/a.test.cpp", '#include "blueberry/lib/shared.hpp"\n')
        self.write("verify/b.test.cpp", '#include <blueberry/lib/other.hpp>\n#include <vector>\n')
        self.write("docs/lib/leaf.md", "old example\n")
        self.write("README.md", "readme\n")
        self.base = self.commit()

    def git(self, *args):
        return subprocess.run(["git", "-C", str(self.root), *args], check=True,
                              text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).stdout.strip()

    def write(self, path, content):
        destination = self.root / path
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_text(content)

    def commit(self):
        self.git("add", "-A")
        self.git("commit", "-qm", "fixture")
        return self.git("rev-parse", "HEAD")

    def plan(self):
        return build_plan(self.root, self.base, self.commit())

    def test_transitive_relative_dependency_and_independent_target(self):
        self.write("blueberry/lib/leaf.hpp", "#pragma once\n// changed\n")
        plan = self.plan()
        self.assertEqual(plan["mode"], "affected")
        self.assertEqual(plan["selected_verify"], ["verify/a.test.cpp"])
        self.assertTrue(plan["compile_required"])
        self.assertIsNone(plan["docs_examples"])
        validate_plan(plan, self.root)

    def test_angle_local_includes_and_new_verifier(self):
        self.write("blueberry/lib/other.hpp", "// changed\n")
        self.write("verify/new.test.cpp", "#include <vector>\n")
        self.assertEqual(self.plan()["selected_verify"], ["verify/b.test.cpp", "verify/new.test.cpp"])

    def test_documentation_ui_has_explicit_empty_selection(self):
        self.write(".verify-helper/docs/static/assets/js/operations.js", "// changed\n")
        plan = self.plan()
        self.assertEqual(plan["selected_verify"], [])
        self.assertFalse(plan["compile_required"])
        self.assertEqual(plan["docs_examples"], [])
        validate_plan(plan, self.root)

    def test_library_docs_select_only_changed_examples(self):
        self.write("docs/lib/leaf.md", "updated example\n")
        self.write(".verify-helper/docs/static/guide.md", "guide\n")
        plan = self.plan()
        self.assertFalse(plan["compile_required"])
        self.assertEqual(plan["selected_verify"], [])
        self.assertEqual(plan["docs_examples"], [".verify-helper/docs/static/guide.md", "docs/lib/leaf.md"])

    def test_deleted_dependency_consumer_preserved_from_base_graph(self):
        (self.root / "blueberry/lib/leaf.hpp").unlink()
        # Keep the consumer but remove the dependency, so the old graph matters.
        self.write("blueberry/lib/shared.hpp", "#pragma once\n")
        plan = self.plan()
        self.assertEqual(plan["selected_verify"], ["verify/a.test.cpp"])
        self.assertTrue(any(c["status"] == "D" for c in plan["changed_files"]))

    def test_rename_uses_old_and_new_paths(self):
        self.git("mv", "blueberry/lib/leaf.hpp", "blueberry/lib/renamed.hpp")
        self.write("blueberry/lib/shared.hpp", '#include "renamed.hpp"\n')
        plan = self.plan()
        self.assertEqual(plan["selected_verify"], ["verify/a.test.cpp"])
        self.assertTrue(any(c.get("previous_path") == "blueberry/lib/leaf.hpp" for c in plan["changed_files"]))

    def test_deleted_verifier_not_scheduled(self):
        (self.root / "verify/a.test.cpp").unlink()
        plan = self.plan()
        self.assertEqual(plan["selected_verify"], [])
        self.assertEqual(plan["all_verify"], ["verify/b.test.cpp"])
        self.assertTrue(plan["compile_required"])

    def test_changed_test_runs_compile_but_not_unrelated_official_cases(self):
        self.write("tests/random/new.cpp", "int main() {}\n")
        plan = self.plan()
        self.assertEqual(plan["selected_verify"], [])
        self.assertTrue(plan["compile_required"])

    def test_unknown_harness_and_workflow_force_full(self):
        for path in ("new.config", "scripts/verify_with_metrics.py", ".github/workflows/verify.yml",
                     ".verify-helper/config.toml", "tests/test_new.py", "requirements-dev.txt"):
            with self.subTest(path=path):
                self.write(path, "changed\n")
                plan = self.plan()
                self.assertEqual(plan["mode"], "full")
                self.assertEqual(plan["selected_verify"], plan["all_verify"])
                self.assertTrue(plan["compile_required"])
                self.git("reset", "--hard", self.base)  # Disposable test repository only.

    def test_missing_base_full_and_explicit_full(self):
        for base in (None, "0" * 40, "missing-reference"):
            with self.subTest(base=base):
                plan = build_plan(self.root, base, "HEAD")
                self.assertEqual(plan["mode"], "full")
                self.assertEqual(plan["selected_verify"], plan["all_verify"])
        self.assertEqual(build_plan(self.root, self.base, "HEAD", force_full=True)["mode"], "full")

    def test_macro_or_missing_include_forces_full(self):
        for source in ('#include HEADER\n', '#include "missing.hpp"\n', '#include_next <vector>\n'):
            with self.subTest(source=source):
                self.write("blueberry/lib/leaf.hpp", source)
                self.assertEqual(self.plan()["mode"], "full")

    def test_comments_continuations_and_cycles(self):
        self.assertEqual(includes('// #include BAD\n/* #include BAD */\n#include \\\n "leaf.hpp" // yes\n'), [('"', "leaf.hpp")])
        self.write("blueberry/lib/leaf.hpp", '#include "shared.hpp"\n')
        self.assertEqual(self.plan()["selected_verify"], ["verify/a.test.cpp"])

    def test_manifest_tampering_rejected(self):
        self.write("blueberry/lib/leaf.hpp", "// changed\n")
        plan = self.plan()
        for key, value in (("selected_verify", []), ("all_verify", []), ("compile_required", False),
                           ("head_sha", self.base), ("docs_examples", []), ("repeats", True)):
            broken = copy.deepcopy(plan)
            broken[key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                validate_plan(broken, self.root)

    def test_pr_merge_base_and_push_before(self):
        self.write("README.md", "changed\n")
        head = self.commit()
        event = {"pull_request": {"base": {"sha": self.base}}}
        self.assertEqual(event_base(self.root, "pull_request", event, head), self.base)
        self.assertEqual(event_base(self.root, "push", {"before": self.base}, head), self.base)
        self.assertIsNone(event_base(self.root, "push", {"before": "0" * 40}, head))
        self.assertIsNone(event_base(self.root, "pull_request", {}, head))

    def test_failed_prior_push_does_not_hide_unverified_changes(self):
        self.write("blueberry/lib/leaf.hpp", "// changed but its CI failed\n")
        failed = self.commit()
        self.write("README.md", "only docs in next push\n")
        head = self.commit()
        runs = [
            {"head_sha": failed, "head_branch": "main", "event": "push", "status": "completed", "conclusion": "failure"},
            {"head_sha": self.base, "head_branch": "main", "event": "push", "status": "completed", "conclusion": "success"},
        ]
        verified, _ = verified_ancestor(self.root, failed, runs)
        self.assertEqual(verified, self.base)
        self.assertEqual(build_plan(self.root, verified, head)["selected_verify"], ["verify/a.test.cpp"])

    def test_untrusted_pr_and_nonancestor_workflow_cannot_supply_baseline(self):
        self.write("README.md", "future\n")
        future = self.commit()
        valid = {"head_sha": self.base, "head_branch": "main", "event": "workflow_dispatch",
                 "status": "completed", "conclusion": "success"}
        for changes in ({"event": "pull_request"}, {"head_branch": "feature"}, {"status": "in_progress"},
                        {"conclusion": "cancelled"}, {"head_sha": future}):
            with self.subTest(changes=changes):
                self.assertEqual(verified_ancestor(self.root, self.base, [dict(valid, **changes)]), (None, None))
        self.assertEqual(verified_ancestor(self.root, self.base, [valid])[0], self.base)

    def test_baseline_api_failure_falls_back_to_full(self):
        with patch.dict("os.environ", {"GITHUB_TOKEN": "test-not-a-secret", "GITHUB_REPOSITORY": "owner/repo"}), \
                patch("scripts.plan_verification.urlopen", side_effect=OSError("offline")):
            base, evidence = trusted_base(self.root, self.base)
        self.assertIsNone(base)
        self.assertIn("API unavailable", evidence["reason"])
        self.assertEqual(build_plan(self.root, base, "HEAD")["mode"], "full")

    def test_cli_dispatch_three_repeats_and_empty_paths_file(self):
        script = Path(__file__).resolve().parents[1] / "scripts/plan_verification.py"
        output = self.root / "plan.json"
        subprocess.run([sys.executable, str(script), "--repo", str(self.root), "--base", self.base,
                        "--event-name", "workflow_dispatch", "--output", str(output)], check=True,
                       stdout=subprocess.PIPE)
        plan = json.loads(output.read_text())
        self.assertEqual((plan["mode"], plan["repeats"]), ("full", 3))
        output.unlink()
        (self.root / "ci-doc-examples.json").unlink()
        subprocess.run([sys.executable, str(script), "--repo", str(self.root), "--base", self.base,
                        "--event-name", "pull_request", "--output", str(output)], check=True,
                       stdout=subprocess.PIPE)
        plan = json.loads(output.read_text())
        self.assertEqual((plan["selected_verify"], plan["repeats"]), ([], 1))
        self.assertEqual(json.loads((self.root / "ci-doc-examples.json").read_text()), [])


class UnitSelectionTests(unittest.TestCase):
    def test_only_known_cpp_integrations_are_removed(self):
        class FakeCase(unittest.TestCase):
            def __init__(self, identifier):
                super().__init__()
                self.identifier = identifier

            def id(self):
                return self.identifier

        identifiers = ["test_tree_random.T.test_run", "test_data_structures.T.test_run",
                       "test_benchmark_runner.BenchmarkRunnerTests.test_static_rmq_implementations_agree",
                       "test_benchmark_runner.BenchmarkRunnerTests.test_manifest_is_valid", "new_test.T.test_run"]
        selected = without_cpp(unittest.TestSuite(FakeCase(i) for i in identifiers))
        self.assertEqual([case.id() for case in selected], identifiers[3:])


class WorkflowGateTests(unittest.TestCase):
    def test_existing_required_jobs_fail_closed_on_planner_failure(self):
        workflow = Path(__file__).resolve().parents[1] / ".github/workflows/verify.yml"
        jobs = yaml.safe_load(workflow.read_text())["jobs"]
        for name in ("compile", "verify"):
            with self.subTest(job=name):
                job = jobs[name]
                self.assertEqual(job["if"], "always()")
                first = job["steps"][0]
                self.assertNotIn("if", first)
                self.assertIn("test '${{ needs.plan.result }}' = success", first["run"])
        self.assertIn("test '${{ needs.compile.result }}' = success", jobs["verify"]["steps"][0]["run"])
        self.assertEqual(jobs["compile"]["name"], "compile (${{ matrix.compiler }}, ${{ matrix.standard }})")
        self.assertEqual(jobs["deploy"]["if"], "github.event_name == 'push' && github.ref == 'refs/heads/main'")

    def test_both_cpp_jobs_use_same_pinned_acl(self):
        workflow = Path(__file__).resolve().parents[1] / ".github/workflows/verify.yml"
        jobs = yaml.safe_load(workflow.read_text())["jobs"]
        refs = []
        for job in ("compile", "verify"):
            checkouts = [s for s in jobs[job]["steps"] if s.get("with", {}).get("repository") == "atcoder/ac-library"]
            self.assertEqual(len(checkouts), 1)
            refs.append(checkouts[0]["with"]["ref"])
        self.assertEqual(refs[0], refs[1])
        self.assertRegex(refs[0], r"^[0-9a-f]{40}$")


if __name__ == "__main__":
    unittest.main()
