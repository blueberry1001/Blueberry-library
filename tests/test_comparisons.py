"""Shared comparison content must match real libraries and finder candidates."""
from pathlib import Path
import re
import unittest
import yaml

ROOT = Path(__file__).resolve().parents[1]
STATIC = ROOT / ".verify-helper/docs/static"


class ComparisonsTest(unittest.TestCase):
    def test_references_and_table_shapes(self):
        families = yaml.safe_load((STATIC / "_data/comparisons.yml").read_text(encoding="utf-8"))
        entries = yaml.safe_load((STATIC / "_data/operations.yml").read_text(encoding="utf-8"))["entries"]
        ids = {entry["id"] for entry in entries}
        self.assertEqual(len(families), len({family["id"] for family in families}))
        for family in families:
            with self.subTest(family=family["id"]):
                self.assertRegex(family["id"], r"^[a-z0-9-]+$")
                self.assertTrue(family["summary"])
                self.assertTrue(family["detail"])
                self.assertNotIn("{{", family["detail"], "Liquid in YAML is not evaluated recursively")
                self.assertLessEqual(family["hints"].keys(), ids)
                for header in family["headers"]:
                    self.assertTrue((ROOT / header).is_file(), header)
                for row in family["rows"]:
                    self.assertEqual(len(row), len(family["columns"]))
                for path in re.findall(r"__BASEURL__(/[^)]+)", family["detail"]):
                    self.assertTrue((ROOT / path.lstrip("/").removesuffix(".html")).with_suffix(".md").exists())

    def test_range_comparison_covers_whole_workload_and_limits(self):
        families = yaml.safe_load((STATIC / "_data/comparisons.yml").read_text(encoding="utf-8"))
        text = next(f for f in families if f["id"] == "range-product")["detail"]
        for fragment in ("U√N + Q", "(U + Q)log N", "N log N + Q", "冪等", "非可換", "max_right", "速度の分岐点ではありません", "DSTを含む4構造の同条件測定", "構築込み"):
            self.assertIn(fragment, text)
