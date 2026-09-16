#!/usr/bin/env python3
"""Equal-input comparison of independently written string transition layouts."""
import hashlib
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[1]
os.chdir(ROOT)
BUILD = ROOT / ".build/lc-expansion-string-benchmark"
BUILD.mkdir(parents=True, exist_ok=True)
compiler = os.environ.get("CXX", "g++")
flags = ["-std=gnu++20", "-O2", "-DNDEBUG", "-Wall", "-Wextra", "-Werror", "-I."]
binary = BUILD / "bench"
subprocess.run([compiler, *flags, "benchmark/string-expansion.cpp", "-o", str(binary)], check=True)
rows = []
for shape in ("random", "repeated", "periodic"):
    modes = ["aho-dense", "aho-sparse", "eertree-dense", "eertree-sparse", "lyndon"]
    for run in range(6):
        order = modes if run % 2 == 0 else list(reversed(modes))
        for mode in order:
            memory = BUILD / "rss.txt"
            completed = subprocess.run(["/usr/bin/time", "-f", "%M", "-o", str(memory), str(binary), mode, shape],
                                       check=True, text=True, capture_output=True)
            row = json.loads(completed.stdout)
            row.update(run=run, warmup=(run == 0), peak_rss_kib=int(memory.read_text().strip()))
            rows.append(row)
            print(shape, mode, run, row["milliseconds"], flush=True)
    for family in ("aho", "eertree"):
        checksums = {r["checksum"] for r in rows if r["shape"] == shape and r["mode"].startswith(family)}
        assert len(checksums) == 1, (shape, family, checksums)
summary = []
for shape in ("random", "repeated", "periodic"):
    for mode in modes:
        group = [r for r in rows if r["shape"] == shape and r["mode"] == mode and not r["warmup"]]
        summary.append(dict(shape=shape, mode=mode,
                            median_ms=statistics.median(r["milliseconds"] for r in group),
                            min_ms=min(r["milliseconds"] for r in group),
                            median_peak_rss_kib=statistics.median(r["peak_rss_kib"] for r in group)))
paths = [Path("benchmark/string-expansion.cpp"), *Path("blueberry/string").glob("*.hpp")]
result = dict(generated_at=datetime.now(timezone.utc).isoformat(),
              compiler=subprocess.check_output([compiler, "--version"], text=True).splitlines()[0],
              flags=flags, platform=platform.platform(), cpu=next((s.split(":", 1)[1].strip() for s in Path("/proc/cpuinfo").read_text().splitlines() if s.startswith("model name")), "unknown"),
              seed=1729, text_length=200000, warmups=1, repeats=5,
              pattern_shapes={"random": "12000 length-32 uniform lowercase strings", "repeated": "one length-100000 all-a string", "periodic": "100 length-4000 substrings of abacaba repeated"},
              timing_scope="Construction, build, count, checksum, destruction; excludes input generation and process startup. RSS includes input and process.",
              source_sha256={p.as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
              summary=summary, runs=rows)
output = ROOT / "benchmark/results/lc-expansion-string-2026-09-17.json"
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(result, indent=2) + "\n")
print(output)
