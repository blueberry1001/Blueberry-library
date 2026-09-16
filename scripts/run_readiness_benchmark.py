#!/usr/bin/env python3
"""Interleave bounded same-input comparisons; preserve every measurement."""
import json
import hashlib
import pathlib
import platform
import re
import statistics
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
FLAGS = ["-std=gnu++20", "-O2", "-DNDEBUG", "-Wall", "-Wextra", "-Werror"]
BASE = "b4695f9"


def main():
    cpu = next((line.split(":", 1)[1].strip() for line in
                pathlib.Path("/proc/cpuinfo").read_text().splitlines()
                if line.startswith("model name")), "unknown")
    result = {"compiler": subprocess.check_output(["g++", "--version"], text=True),
              "flags": FLAGS, "cpu": cpu, "platform": platform.platform(),
              "baseline_revision": BASE, "seed": 1729, "repeats": 5, "warmups": 1,
              "dijkstra": {"vertices": 100000, "edges": 500000},
              "rectangles": {"grid_side": 1024, "points": 30000, "updates_and_queries": 60000},
              "runs": [], "summary": []}
    result["source_sha256"] = {path: hashlib.sha256((ROOT / path).read_bytes()).hexdigest()
        for path in ("benchmark/contest-readiness.cpp", "blueberry/graph/dijkstra.hpp",
                     "blueberry/data-structure/offline-fenwick-tree-2d.hpp")}
    with tempfile.TemporaryDirectory(prefix="blueberry-readiness-") as temporary:
        temporary = pathlib.Path(temporary)
        baseline = subprocess.check_output(
            ["git", "show", f"{BASE}:blueberry/graph/dijkstra.hpp"], cwd=ROOT, text=True)
        (temporary / "baseline-dijkstra.hpp").write_text(baseline)
        for variant in ["baseline", "current"]:
            subprocess.run(["g++", *FLAGS, "-I.", "-I" + str(temporary),
                            *(["-DBLUEBERRY_BASELINE_DIJKSTRA"] if variant == "baseline" else []),
                            "benchmark/contest-readiness.cpp", "-o", str(temporary / variant)],
                           cwd=ROOT, check=True)
        groups = [[("baseline", "dijkstra"), ("current", "dijkstra")],
                  [("current", "dense"), ("current", "compressed")]]
        for group in groups:
            checksums = set()
            for repeat in range(6):
                # Alternate order to reduce systematic warm-cache/order bias.
                order = group if repeat % 2 == 0 else list(reversed(group))
                for variant, workload in order:
                    run = subprocess.run(["/usr/bin/time", "-f", "peak_rss_kib=%M",
                                          str(temporary / variant), workload],
                                         cwd=ROOT, capture_output=True, text=True, check=True)
                    fields = dict(re.findall(r"(\w+)=([0-9.]+)", run.stdout + run.stderr))
                    checksums.add(fields["checksum"])
                    if repeat:
                        result["runs"].append({"variant": variant, "workload": workload,
                            "repeat": repeat, "build_ms": float(fields["build_ms"]),
                            "operation_ms": float(fields["operation_ms"]),
                            "peak_rss_kib": int(fields["peak_rss_kib"]),
                            "checksum": fields["checksum"], "stdout": run.stdout, "stderr": run.stderr})
            if len(checksums) != 1:
                raise RuntimeError(f"checksum mismatch: {group}: {checksums}")
            for variant, workload in group:
                runs = [row for row in result["runs"]
                        if row["variant"] == variant and row["workload"] == workload]
                result["summary"].append({"variant": variant, "workload": workload,
                    "median_operation_ms": statistics.median(row["operation_ms"] for row in runs),
                    "min_operation_ms": min(row["operation_ms"] for row in runs),
                    "median_build_ms": statistics.median(row["build_ms"] for row in runs),
                    "median_peak_rss_kib": statistics.median(row["peak_rss_kib"] for row in runs)})
    output = ROOT / "benchmark/results/contest-readiness-2026-09-16.json"
    output.write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps(result["summary"], indent=2))


if __name__ == "__main__":
    main()
