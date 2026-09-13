import json
from pathlib import Path
import subprocess
import tempfile
import unittest

from scripts.run_benchmarks import (
    load_manifest,
    median_absolute_deviation,
    parse_benchmark_output,
    validate_checksums,
)


ROOT = Path(__file__).resolve().parents[1]


class BenchmarkRunnerTests(unittest.TestCase):
    def test_manifest_is_valid(self):
        manifest = load_manifest(ROOT / "benchmark/manifest.json")
        self.assertEqual(manifest["schema"], 1)
        self.assertGreaterEqual(len(manifest["targets"]), 3)

    def test_median_absolute_deviation(self):
        self.assertEqual(median_absolute_deviation([1.0, 2.0, 100.0]), 1.0)

    def test_output_requires_one_json_line_and_checksum(self):
        self.assertEqual(parse_benchmark_output('{"checksum": 12}\n', "x")["checksum"], 12)
        for output in ("", "{}\n", "{}\n{}\n", "not json\n", '{"checksum": 1, "query_seconds": -1}\n'):
            with self.subTest(output=output), self.assertRaises(RuntimeError):
                parse_benchmark_output(output, "x")

    def test_checksum_validation(self):
        target = {"name": "a", "group": "g"}
        jobs = [
            {"target": target, "flag_set": "f", "samples": [{"checksum": 1}, {"checksum": 1}]},
            {"target": {"name": "b", "group": "g"}, "flag_set": "f", "samples": [{"checksum": 1}]},
        ]
        validate_checksums(jobs)
        jobs[1]["samples"][0]["checksum"] = 2
        with self.assertRaises(RuntimeError):
            validate_checksums(jobs)

    def test_static_rmq_implementations_agree(self):
        source = ROOT / "benchmark/static_rmq_benchmark.cpp"
        with tempfile.TemporaryDirectory() as temporary:
            checksums = {}
            for implementation in range(3):
                binary = Path(temporary) / f"rmq-{implementation}"
                subprocess.run(
                    ["g++", "-std=c++20", "-O1", "-DNDEBUG", "-I", str(ROOT),
                     f"-DBENCH_IMPL={implementation}", str(source), "-o", str(binary)],
                    check=True,
                )
                for workload in ("random", "small"):
                    result = subprocess.run(
                        [str(binary), "257", "2000", "2", "123456", workload],
                        check=True, text=True, stdout=subprocess.PIPE,
                    )
                    checksums.setdefault(workload, set()).add(json.loads(result.stdout)["checksum"])
            self.assertEqual({key: len(value) for key, value in checksums.items()}, {"random": 1, "small": 1})


if __name__ == "__main__":
    unittest.main()
