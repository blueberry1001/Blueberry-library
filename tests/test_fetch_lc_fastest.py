import importlib.util
import io
import json
from pathlib import Path
import unittest
from unittest import mock


SCRIPT = Path(__file__).resolve().parents[1] / "scripts" / "fetch_lc_fastest.py"
SPEC = importlib.util.spec_from_file_location("fetch_lc_fastest", SCRIPT)
assert SPEC and SPEC.loader
lc = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(lc)


class FakeResponse(io.BytesIO):
    def __enter__(self):
        return self

    def __exit__(self, *_):
        self.close()


class FetchFastestTest(unittest.TestCase):
    def test_fetches_fastest_rows_and_profiles_source(self):
        requests = []

        def fake_urlopen(request, timeout):
            requests.append((request.full_url, timeout))
            if request.full_url.startswith("https://example.test/submissions?"):
                payload = {
                    "count": 1,
                    "submissions": [{
                        "id": 42,
                        "user_name": "tester",
                        "lang": "C++23",
                        "time": 0.038,
                        "memory": 123,
                    }],
                }
            else:
                payload = {
                    "overview": {"id": 42},
                    "source": "#pragma GCC target(\"avx2\")\nvoid ntt() {}\n",
                    "case_results": [{"time": 0.038}],
                }
            return FakeResponse(json.dumps(payload).encode())

        with mock.patch.object(lc, "urlopen", side_effect=fake_urlopen):
            rows = lc.fetch_fastest(
                "exp_of_formal_power_series", limit=1, base_url="https://example.test"
            )

        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]["id"], 42)
        self.assertEqual(rows[0]["case_max_seconds"], 0.038)
        self.assertTrue(rows[0]["profile"]["uses_ntt"])
        self.assertTrue(rows[0]["profile"]["uses_simd"])
        self.assertEqual(len(requests), 2)
        self.assertIn("order=%2Btime", requests[0][0])
        self.assertIn("status=AC", requests[0][0])
        self.assertEqual(requests[0][1], 30)

    def test_report_uses_milliseconds(self):
        profile = {key: False for key in (
            "uses_ntt", "uses_simd", "has_target_pragma",
            "uses_acl_convolution", "uses_fast_io",
        )}
        profile.update({"lines": 1, "bytes": 2})
        rows = [{
            "id": 7,
            "overview": {
                "user_name": None,
                "lang": "C++23",
                "time": 0.125,
                "memory": 456,
            },
            "case_max_seconds": 0.125,
            "profile": profile,
        }]
        report = lc.make_report("demo", rows, "https://example.test")
        self.assertIn("125.0 ms", report)
        self.assertIn("(anonymous)", report)


if __name__ == "__main__":
    unittest.main()
