import importlib.util
from pathlib import Path
import tempfile
import unittest
import json
import shutil
import subprocess

MODULE = Path(__file__).resolve().parents[1] / "scripts/library_checker_coverage.py"
spec = importlib.util.spec_from_file_location("coverage", MODULE)
coverage = importlib.util.module_from_spec(spec)
spec.loader.exec_module(coverage)


class CoverageTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.write("blueberry/ds/a.hpp", '#include "blueberry/ds/b.hpp"\n')
        self.write("blueberry/ds/b.hpp", "// header\n")
        self.catalog = {"blueberry/ds/a.hpp": "A", "blueberry/ds/b.hpp": "B"}
        self.inventory = coverage.make_inventory(
            [{"name": "alpha", "title": "Alpha"}, {"name": "beta", "title": "Beta"}],
            {"categories": [{"name": "Data Structure", "problems": ["alpha"]}]},
            "a" * 40, "2026-09-17T00:00:00Z")

    def write(self, path, text):
        target = self.root / path
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(text, encoding="utf-8")

    def driver(self, name="one", problem="alpha", header="blueberry/ds/a.hpp", prefix=""):
        self.write(f"verify/{name}.test.cpp", prefix + f'#define PROBLEM "https://judge.yosupo.jp/problem/{problem}"\n#include "{header}"\n')

    def rows(self, mappings=None):
        return coverage.build_rows(self.root, self.inventory, mappings or {}, self.catalog)

    def test_inventory_preserves_uncategorized_published_problem(self):
        self.assertEqual(len(self.inventory["problems"]), 2)
        self.assertEqual(self.inventory["problems"][1]["category"], "Uncategorized")

    def test_duplicate_or_invalid_problem_id_rejected(self):
        for problems in ([{"name": "a", "title": "A"}] * 2, [{"name": "../a", "title": "A"}]):
            with self.assertRaises(ValueError):
                coverage.make_inventory(problems, {"categories": []}, "a" * 40, "date")

    def test_multiple_drivers_and_transitive_dependencies(self):
        self.driver()
        self.driver("two")
        row = self.rows()[0]
        self.assertEqual(row["status"], "verify")
        self.assertEqual(len(row["verifies"]), 2)
        self.assertEqual([item["path"] for item in row["headers"]], sorted(self.catalog))
        self.assertNotIn("passed", row)

    def test_ignored_driver_does_not_claim_coverage(self):
        self.driver(prefix="#define IGNORE\n")
        self.assertEqual(self.rows()[0]["status"], "missing")
        self.assertEqual(self.rows()[0]["ignored_verifies"], ["verify/one.test.cpp"])

    def test_comment_does_not_create_a_driver(self):
        self.write("verify/comment.test.cpp", '// #define PROBLEM "https://judge.yosupo.jp/problem/unknown"\n')
        self.assertEqual(self.rows()[0]["status"], "missing")

    def test_unknown_problem_and_missing_dependency_are_errors(self):
        self.driver(problem="unknown")
        with self.assertRaises(ValueError):
            self.rows()
        self.driver(header="blueberry/ds/missing.hpp")
        with self.assertRaises(ValueError):
            self.rows()

    def test_umbrella_smoke_is_not_all_library_coverage(self):
        self.write("blueberry/all.hpp", '#include "blueberry/ds/a.hpp"\n')
        self.driver(header="blueberry/all.hpp")
        self.assertEqual(self.rows()[0]["status"], "driver")
        self.assertEqual(self.rows()[0]["headers"], [])

    def test_manual_api_and_acl_are_distinct(self):
        rows = self.rows({"alpha": {"status": "implemented", "headers": ["blueberry/ds/a.hpp"], "note": "Use A"},
                          "beta": {"status": "acl", "url": "https://atcoder.github.io/ac-library/production/document_en/segtree.html", "note": "Use ACL"}})
        self.assertEqual([r["status"] for r in rows], ["implemented", "acl"])
        self.assertEqual(rows[1]["headers"], [])
        with self.assertRaises(ValueError):
            self.rows({"alpha": {"status": "implemented", "headers": ["blueberry/ds/no.hpp"], "note": "missing"}})
        with self.assertRaises(ValueError):
            self.rows({"unknown": {"status": "acl", "url": "https://example.com", "note": "unknown"}})

    def test_snapshot_and_markdown_have_every_problem_once(self):
        self.driver()
        document = coverage.render_markdown(self.inventory, self.rows())
        self.assertEqual(document.count("https://judge.yosupo.jp/problem/alpha"), 1)
        self.assertEqual(document.count("https://judge.yosupo.jp/problem/beta"), 1)
        self.assertIn("[x]", document)
        self.assertIn("[ ]", document)
        self.assertIn("AC", document)

    def test_cycles_and_stale_generated_outputs(self):
        self.write("blueberry/ds/b.hpp", '#include "blueberry/ds/a.hpp"\n')
        self.driver()
        self.assertEqual(len(self.rows()[0]["headers"]), 2)
        target = self.root / "generated.md"
        coverage.write_or_check(target, "first\n", False)
        coverage.write_or_check(target, "first\n", True)
        with self.assertRaises(ValueError):
            coverage.write_or_check(target, "changed\n", True)
        self.assertEqual(target.read_text(), "first\n")

    def test_browser_filter_keeps_implementation_distinct_from_acl(self):
        node = shutil.which("node")
        if node is None:
            self.skipTest("Node.js is required to run JavaScript filter tests")
        script = r'''
const {matches} = require('./.verify-helper/docs/static/assets/js/coverage.js');
const row = {category:'Data Structure',status:'implemented',text:'Static Range Sum Fenwick Tree'};
console.log(JSON.stringify([
  matches(row, {search:'ＦＥＮＷＩＣＫ sum'}),
  matches(row, {status:'verify'}),
  matches(row, {status:'acl'}),
  matches(row, {status:'implemented',category:'Data Structure',search:'sum'}),
  matches(row, {category:'Math'}),
  matches(row, {search:'sum absent'})
]));
'''
        result = subprocess.run([node, "-e", script], cwd=MODULE.parents[1], capture_output=True, text=True, check=True)
        self.assertEqual(json.loads(result.stdout), [True, False, False, True, False, False])


if __name__ == "__main__":
    unittest.main()
