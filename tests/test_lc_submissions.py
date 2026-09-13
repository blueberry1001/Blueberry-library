import json
from pathlib import Path
import tempfile
import unittest

from scripts.fetch_lc_submissions import (
    LibraryCheckerClient,
    fetch_problem,
    markdown_report,
    public_detail,
    source_profile,
)


class FakeClient:
    def get_json(self, path, params=None):
        if path == "/submissions":
            row = {"id": 10 if params.get("user") else 1, "time": 0.1 if params.get("user") else 0.02,
                   "memory": 200 if params.get("user") else 100, "user_name": params.get("user")}
            return {"count": 1, "submissions": [row]}
        submission_id = int(path.rsplit("/", 1)[1])
        return {"overview": {"id": submission_id}, "source": "#include <iostream>\nint main(){}\n",
                "case_results": [{"time": 0.1, "memory": 200}]}


class LibraryCheckerResearchTests(unittest.TestCase):
    def test_source_profile(self):
        profile = source_profile(
            "#include <vector>\n#include <cstdio>\n"
            "struct BlockedSparseTable{}; int main(){ fread(0,0,0,stdin); __builtin_clz(1); }\n"
        )
        self.assertTrue(profile["uses_buffered_c_io"])
        self.assertTrue(profile["mentions_blocked_rmq"])
        self.assertTrue(profile["uses_bit_scan"])
        self.assertFalse(profile["uses_nested_vector"])
        self.assertEqual(len(profile["sha256"]), 64)

    def test_namespaced_nested_vector_is_detected(self):
        self.assertTrue(source_profile("std::vector<std::vector<int>> table;")["uses_nested_vector"])

    def test_public_detail_omits_source(self):
        detail = public_detail({
            "overview": {"id": 1}, "source": "secret", "case_results": [{"time": 0.2, "memory": 3}]
        })
        self.assertNotIn("source", detail)
        self.assertEqual(detail["max_case_seconds"], 0.2)

    def test_problem_ratios_and_markdown(self):
        item = fetch_problem(
            FakeClient(), "staticrmq", language="cpp", limit=5, user="example",
            analyze_top=1, source_dir=None,
        )
        self.assertEqual(item["time_ratio_user_to_fastest"], 5)
        self.assertEqual(item["memory_ratio_user_to_fastest"], 2)
        report = {"generated_at": "now", "problems": [item]}
        markdown = markdown_report(report)
        self.assertIn("5.000x", markdown)
        self.assertIn("https://judge.yosupo.jp/submission/1", markdown)

    def test_fresh_cache_is_used(self):
        with tempfile.TemporaryDirectory() as temporary:
            client = LibraryCheckerClient("https://example.invalid", cache_dir=Path(temporary))
            url = client._url("/x", {"b": 2, "a": 1})
            cache = client._cache_path(url)
            cache.write_text(json.dumps({"data": {"cached": True}}))
            self.assertEqual(client.get_json("/x", {"a": 1, "b": 2}), {"cached": True})


if __name__ == "__main__":
    unittest.main()
