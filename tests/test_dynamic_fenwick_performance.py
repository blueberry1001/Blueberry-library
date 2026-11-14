import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from scripts.check_dynamic_fenwick_performance import SHAPES, assess, digest, main


class DynamicFenwickPerformanceTest(unittest.TestCase):
    def fixture(self, root):
        candidate, driver = root / "candidate.hpp", root / "driver.cpp"
        candidate.write_text("candidate", encoding="utf-8")
        driver.write_text("driver", encoding="utf-8")
        result = {"count": 300000, "runs": 3, "seed": 1729,
                  "flags": ["-std=gnu++20", "-O2", "-DNDEBUG"],
                  "compiler": "g++ test", "platform": "test", "cpu": "test",
                  "sha256": {"candidate": digest(candidate), "driver": digest(driver),
                             "nyaan": "reference", "nyaan_hash": "hash"}, "measurements": []}
        for shape in SHAPES:
            for variant in ("candidate", "nyaan"):
                for run, elapsed in enumerate((100, 101, 99), 1):
                    result["measurements"].append({"shape": shape, "variant": variant, "run": run,
                                                   "milliseconds": elapsed, "peak_rss_kib": 123,
                                                   "checksum": 12})
        return result, candidate, driver

    def test_median_comparison_and_regression_threshold(self):
        with tempfile.TemporaryDirectory() as temporary, \
             patch("scripts.check_dynamic_fenwick_performance.EXPECTED_REFERENCE",
                   {"nyaan": "reference", "nyaan_hash": "hash"}):
            result, candidate, driver = self.fixture(Path(temporary))
            self.assertEqual(assess(result, candidate, driver)["status"], "passed")
            for sample in result["measurements"]:
                if sample["shape"] == SHAPES[0] and sample["variant"] == "candidate":
                    sample["milliseconds"] *= 2.1
            assessment = assess(result, candidate, driver)
            self.assertEqual(assessment["status"], "failed")
            self.assertGreater(assessment["rows"][0]["ratio"], 2)

    def test_missing_duplicate_bad_checksum_and_invalid_timing_never_pass(self):
        with tempfile.TemporaryDirectory() as temporary, \
             patch("scripts.check_dynamic_fenwick_performance.EXPECTED_REFERENCE",
                   {"nyaan": "reference", "nyaan_hash": "hash"}):
            result, candidate, driver = self.fixture(Path(temporary))
            for update in ("missing", "duplicate", "checksum", "negative", "nonfinite", "rss", "reference"):
                with self.subTest(update=update):
                    broken = json.loads(json.dumps(result))
                    if update == "missing": broken["measurements"].pop()
                    if update == "duplicate": broken["measurements"][-1] = broken["measurements"][0]
                    if update == "checksum": broken["measurements"][0]["checksum"] = 99
                    if update == "negative": broken["measurements"][0]["milliseconds"] = -1
                    if update == "nonfinite": broken["measurements"][0]["milliseconds"] = float("nan")
                    if update == "rss": broken["measurements"][0]["peak_rss_kib"] = 0
                    if update == "reference": broken["sha256"]["nyaan"] = "other"
                    with self.assertRaises(ValueError):
                        assess(broken, candidate, driver)

    def test_unavailable_results_are_written_as_unmeasured_and_fail(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            _, candidate, driver = self.fixture(root)
            output = root / "assessment.json"
            code = main(["--results", str(root / "missing.json"), "--candidate", str(candidate),
                         "--driver", str(driver), "--output", str(output)])
            self.assertEqual(code, 1)
            self.assertEqual(json.loads(output.read_text(encoding="utf-8"))["status"], "not_measured")


if __name__ == "__main__":
    unittest.main()
