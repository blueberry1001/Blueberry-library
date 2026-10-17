#!/usr/bin/env python3
"""Build public submissions separately; retain errors instead of inventing times."""
import json
from pathlib import Path
import statistics
import subprocess
import sys
import time

root = Path(__file__).resolve().parents[2]
out = Path(__file__).resolve().parent
records = []
cases = [("bernoulli", None), ("bernoulli", 17797), ("bernoulli", 65329),
         ("bell", None), ("bell", 406022), ("partition", None), ("partition", 207699)]
own_only = "--own-only" in sys.argv
if own_only:
    cases = [case for case in cases if case[1] is None]
for name, submission in cases:
    folder = "fastest-bernoulli-cpp" if name == "bernoulli" else "fastest-" + name
    source = root / ".build/fps-sequences/research-sources" / folder / f"submission-{submission}.cpp" if submission else root / f"verify/math/{name}.test.cpp"
    tag = str(submission) if submission else "own-" + name
    binary = root / f".build/fps-sequences/fastest-{tag}"
    command = ["g++", "-std=gnu++20", "-O2", "-DNDEBUG", "-I.", "-isystem", ".deps/ac-library", str(source), "-o", str(binary)]
    build = subprocess.run(command, text=True, capture_output=True)
    (out / f"compile-{tag}.log").write_text(build.stdout + build.stderr)
    row = {"name": name, "submission": submission, "command": command,
           "compile_returncode": build.returncode, "samples": []}
    if build.returncode == 0:
        for n in [8192, 131072, 500000]:
            for repeat in range(3):
                begin = time.perf_counter()
                run = subprocess.run(["/usr/bin/time", "-f", "%e,%M", str(binary)],
                                     input=str(n) + "\n", text=True, capture_output=True)
                sample = {"n": n, "repeat": repeat, "returncode": run.returncode,
                          "process_ms": (time.perf_counter() - begin) * 1000,
                          "time_stderr": run.stderr}
                if run.returncode == 0:
                    values = [int(s) for s in run.stdout.split()]
                    expected_run = subprocess.run([str(root / ".build/fps-sequences/compare"), name, str(n)],
                                                  text=True, capture_output=True, check=True)
                    expected_checksum = int(expected_run.stdout.strip().split(",")[6])
                    sample["tokens"] = len(values)
                    sample["checksum"] = sum((i + 1) * v for i, v in enumerate(values)) % (1 << 64)
                    sample["matches"] = len(values) == n + 1 and sample["checksum"] == expected_checksum
                    if not sample["matches"]:
                        raise RuntimeError(f"source {submission} output did not match")
                row["samples"].append(sample)
    records.append(row)
    (out / ("own-fastio.json" if own_only else "fastest-local.json")).write_text(json.dumps(records, indent=2) + "\n")
