import hashlib
from pathlib import Path
import tempfile
import unittest

from scripts.audit_verify_limits import audit, official_limits, render


class TimeLimitAuditTest(unittest.TestCase):
    def test_screening_preserves_missing_and_changed_measurements(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "verify").mkdir()
            source = b'#define PROBLEM "https://judge.yosupo.jp/problem/example"\n'
            for name in ("high", "medium", "low", "missing", "failed", "changed", "unknown"):
                (root / "verify" / f"{name}.test.cpp").write_bytes(source)
            good_hash = hashlib.sha256(source).hexdigest()
            rows = []
            for name, value, status, hash_value in (
                    ("high", 5, "passed", good_hash), ("medium", 2.5, "passed", good_hash),
                    ("low", 2, "passed", good_hash), ("failed", 0, "failed", good_hash),
                    ("changed", 6, "passed", "outdated"), ("unknown", None, "passed", good_hash)):
                row = {"path": f"verify/{name}.test.cpp", "status": status,
                       "environment_index": 0, "verifier_sha256": hash_value}
                if value is not None:
                    row["worst_case_seconds"] = value
                rows.append(row)
            limits = {"example": {"seconds": 5.0, "path": "ds/example/info.toml", "sha256": "hash"}}
            report = {"schema": 1, "results": rows}
            found = {row["path"]: row for row in audit(report, root, limits)}
            self.assertEqual(found["verify/high.test.cpp"]["assessment"], "at_or_above_limit")
            self.assertEqual(found["verify/medium.test.cpp"]["assessment"], "review_headroom")
            self.assertEqual(found["verify/low.test.cpp"]["assessment"], "below_half")
            for name in ("missing", "failed", "unknown"):
                self.assertEqual(found[f"verify/{name}.test.cpp"]["assessment"], "unmeasured")
                self.assertIsNone(found[f"verify/{name}.test.cpp"]["ratio"])
            self.assertEqual(found["verify/changed.test.cpp"]["assessment"], "source_changed_or_unknown")
            self.assertIsNone(found["verify/changed.test.cpp"]["problem"])
            text = render(report, list(found.values()), root, None, None, None, None)
            self.assertIn("not the judge time limit", text)
            self.assertIn("No row establishes an AC or TLE verdict", text)

    def test_official_limits_only_and_ambiguous_names(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            example = root / "data_structure/example/info.toml"
            example.parent.mkdir(parents=True)
            example.write_text('title = "Example"\ntimelimit = 5.0\n[[tests]]\ntimelimit = 1\n')
            limits = official_limits(root)
            self.assertEqual(limits["example"]["seconds"], 5.0)
            self.assertEqual(limits["example"]["path"], "data_structure/example/info.toml")
            duplicate = root / "other/example/info.toml"
            duplicate.parent.mkdir(parents=True)
            duplicate.write_text("timelimit = 3.0\n")
            with self.assertRaises(ValueError):
                official_limits(root)

    def test_unknown_problem_limit_and_nonfinite_time(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "verify").mkdir()
            source = b'#define PROBLEM "https://judge.yosupo.jp/problem/absent"\n'
            (root / "verify/a.test.cpp").write_bytes(source)
            row = {"path": "verify/a.test.cpp", "status": "passed", "environment_index": 0,
                   "verifier_sha256": hashlib.sha256(source).hexdigest(), "worst_case_seconds": 1}
            report = {"schema": 1, "results": [row]}
            self.assertEqual(audit(report, root, {})[0]["assessment"], "limit_unknown")
            row["worst_case_seconds"] = float("nan")
            self.assertEqual(audit(report, root, {})[0]["assessment"], "unmeasured")


if __name__ == "__main__":
    unittest.main()
