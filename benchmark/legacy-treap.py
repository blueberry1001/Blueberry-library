#!/usr/bin/env python3
"""Run only in a coordinated quiet window; preserve all comparative samples."""
import hashlib
import json
import pathlib
import platform
import statistics
import subprocess
from datetime import datetime, timezone

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "benchmark/results/legacy-treap"
OUTPUT.mkdir(parents=True, exist_ok=True)
BINARY = ROOT / ".build/legacy-treap/benchmark"
FLAGS = ["-std=gnu++20", "-O2", "-DNDEBUG", "-Wall", "-Wextra", "-Werror", "-I.", "-isystem", ".deps/ac-library"]
subprocess.run(["g++", *FLAGS, "benchmark/legacy-treap.cpp", "-o", str(BINARY)], cwd=ROOT, check=True)
completed = subprocess.run([str(BINARY)], cwd=ROOT, text=True, capture_output=True)
(OUTPUT / "raw.tsv").write_text(completed.stdout)
(OUTPUT / "stderr.txt").write_text(completed.stderr)
completed.check_returncode()
rows = []
for line in completed.stdout.splitlines():
    name, variant, repeat, ns, checksum = line.split("\t")
    rows.append(dict(name=name, variant=variant, repeat=int(repeat), ns=int(ns), checksum=int(checksum)))
summary = []
for name, variant in sorted({(row["name"], row["variant"]) for row in rows}):
    samples = [row["ns"] for row in rows if row["name"] == name and row["variant"] == variant and row["repeat"] >= 0]
    summary.append(dict(name=name, variant=variant, median_ns=statistics.median(samples), min_ns=min(samples), samples=samples))
paths = ["benchmark/legacy-treap.cpp", "blueberry/data-structure/implicit-treap.hpp", "blueberry/data-structure/ordered-multiset.hpp"]
report = dict(generated_at=datetime.now(timezone.utc).isoformat(),
              compiler=subprocess.check_output(["g++", "--version"], text=True).splitlines()[0], flags=FLAGS,
              platform=platform.platform(), cpu=subprocess.check_output(["lscpu"], text=True),
              seed=20260917, repeats=5, warmups=1,
              sha256={path:hashlib.sha256((ROOT / path).read_bytes()).hexdigest() for path in paths},
              load_note="Agent compilation and official verification paused by coordination; normal desktop background activity may remain.",
              summary=summary)
(OUTPUT / "results.json").write_text(json.dumps(report, indent=2) + "\n")
print(json.dumps(summary, indent=2))
