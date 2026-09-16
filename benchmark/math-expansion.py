#!/usr/bin/env python3
"""Compile and preserve equal-input mathematical algorithm comparisons."""
import hashlib
import json
import pathlib
import platform
import statistics
import subprocess
from datetime import datetime, timezone

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "benchmark/results/lc-expansion-math"
OUTPUT.mkdir(parents=True, exist_ok=True)
BINARY = ROOT / ".build/lc-expansion-math/benchmark"
FLAGS = ["-std=gnu++20", "-O2", "-DNDEBUG", "-Wall", "-Wextra", "-Werror",
         "-I.", "-isystem", ".deps/ac-library"]
subprocess.run(["g++", *FLAGS, "benchmark/math-expansion.cpp", "-o", str(BINARY)],
               cwd=ROOT, check=True)
completed = subprocess.run([str(BINARY)], cwd=ROOT, text=True, capture_output=True)
(OUTPUT / "raw.tsv").write_text(completed.stdout)
(OUTPUT / "stderr.txt").write_text(completed.stderr)
completed.check_returncode()
rows = []
for line in completed.stdout.splitlines():
    name, variant, repeat, ns, checksum = line.split("\t")
    rows.append(dict(name=name, variant=variant, repeat=int(repeat),
                     nanoseconds=int(ns), checksum=int(checksum)))
summary = []
for name, variant in sorted({(row["name"], row["variant"]) for row in rows}):
    samples = [row["nanoseconds"] for row in rows
               if row["name"] == name and row["variant"] == variant and row["repeat"] >= 0]
    summary.append(dict(name=name, variant=variant, median_ns=statistics.median(samples),
                        min_ns=min(samples), samples=samples))
paths = [ROOT / "benchmark/math-expansion.cpp", *(ROOT / "blueberry/math" / f"{n}.hpp"
         for n in ("factorize", "mod-sqrt", "linear-recurrence"))]
report = dict(generated_at=datetime.now(timezone.utc).isoformat(),
              compiler=subprocess.check_output(["g++", "--version"], text=True).splitlines()[0],
              flags=FLAGS, platform=platform.platform(),
              cpu=subprocess.check_output(["lscpu"], text=True),
              seed=20260917, warmups=1, repeats=5, returncode=completed.returncode,
              sha256={p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
                      for p in paths}, summary=summary,
              load_note="Shared workstation; official verification may run concurrently. Interpret only large differences, not tiny percentage changes.")
(OUTPUT / "results.json").write_text(json.dumps(report, indent=2) + "\n")
print(json.dumps(summary, indent=2))
