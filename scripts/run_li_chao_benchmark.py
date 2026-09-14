#!/usr/bin/env python3
"""Run both independently implemented Li Chao candidates on identical inputs."""
import json
import pathlib
import platform
import re
import statistics
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "benchmark/results/li-chao-tree-2026-09-14.json"
FLAGS = ["-std=c++20", "-O2", "-DNDEBUG", "-Wall", "-Wextra", "-Werror", "-I."]


def main():
    cpu = next((line.split(":", 1)[1].strip() for line in
                pathlib.Path("/proc/cpuinfo").read_text().splitlines()
                if line.startswith("model name")), "unknown")
    result = {"compiler": subprocess.check_output(["g++", "--version"], text=True),
              "flags": FLAGS, "platform": platform.platform(), "cpu": cpu,
              "n": 100000, "operations": 200000, "seed": 1729,
              "repeats": 5, "warmups": 1, "results": []}
    with tempfile.TemporaryDirectory(prefix="blueberry-li-chao-") as temporary:
        executable = str(pathlib.Path(temporary) / "benchmark")
        subprocess.run(["g++", *FLAGS, "benchmark/li-chao-tree.cpp", "-o", executable],
                       cwd=ROOT, check=True)
        for workload in ["line", "segment"]:
            checksums = set()
            for candidate in ["compressed", "dynamic"]:
                command = ["/usr/bin/time", "-f", "peak_rss_kib=%M", executable,
                           candidate, workload, str(result["n"]), str(result["seed"])]
                subprocess.run(command, cwd=ROOT, check=True, capture_output=True)
                runs = []
                for _ in range(result["repeats"]):
                    run = subprocess.run(command, cwd=ROOT, check=True, capture_output=True, text=True)
                    fields = dict(re.findall(r"(\w+)=([0-9.]+)", run.stdout + run.stderr))
                    checksums.add(fields["checksum"])
                    runs.append({"milliseconds": float(fields["milliseconds"]),
                                 "peak_rss_kib": int(fields["peak_rss_kib"]),
                                 "checksum": fields["checksum"],
                                 "stdout": run.stdout, "stderr": run.stderr})
                result["results"].append({"candidate": candidate, "workload": workload,
                    "command": ["benchmark/li-chao-tree.cpp (compiled)", *command[4:]],
                    "runs": runs,
                    "median_ms": statistics.median(run["milliseconds"] for run in runs),
                    "min_ms": min(run["milliseconds"] for run in runs),
                    "median_peak_rss_kib": statistics.median(run["peak_rss_kib"] for run in runs)})
            if len(checksums) != 1:
                raise RuntimeError(f"checksum mismatch for {workload}: {checksums}")
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text(json.dumps(result, indent=2) + "\n")
    print(OUTPUT)


if __name__ == "__main__":
    main()
