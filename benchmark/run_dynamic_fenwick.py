#!/usr/bin/env python3
"""Compare legacy, online AVL, and pre-registered offline Fenwick on equal inputs."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--output", type=Path, default=ROOT / "benchmark/results/dynamic-fenwick.json")
parser.add_argument("--repeats", type=int, default=5)
parser.add_argument("--count", type=int, default=60000)
args = parser.parse_args()
os.chdir(ROOT)
build = ROOT / ".build/legacy-fenwick/benchmark"
build.mkdir(parents=True, exist_ok=True)
baseline = "03ee56684d0448db62b72a5b542861cc6fc35c28"
legacy = subprocess.check_output(["git", "show", f"{baseline}:blueberry/DynamicFenwickTree2D.hpp"])
(build / "legacy-dynamic-fenwick-tree-2d.hpp").write_bytes(legacy)
compiler = os.environ.get("CXX", "g++")
flags = ["-std=gnu++20", "-O2", "-DNDEBUG", "-I.", "-I" + str(build)]
binary = build / "benchmark"
subprocess.run([compiler, *flags, "benchmark/dynamic-fenwick-tree.cpp", "-o", str(binary)], check=True)
rows = []
for shape in ("bounded", "repeated", "huge"):
    modes = ["online", "offline"] + ([] if shape == "huge" else ["legacy"])
    for run in range(args.repeats + 1):
        for mode in modes if run % 2 == 0 else list(reversed(modes)):
            rss = build / "rss.txt"
            completed = subprocess.run(["/usr/bin/time", "-f", "%M", "-o", str(rss),
                                        str(binary), mode, shape, str(args.count)], text=True,
                                       capture_output=True)
            if completed.returncode:
                # Preserve failures; never replace a missing measurement with zero.
                rows.append(dict(shape=shape, mode=mode, run=run, failed=True,
                                 returncode=completed.returncode, stderr=completed.stderr))
                args.output.parent.mkdir(parents=True, exist_ok=True)
                args.output.write_text(json.dumps(dict(runs=rows), indent=2) + "\n")
                raise RuntimeError(completed.stderr)
            row = json.loads(completed.stdout)
            row.update(run=run, warmup=(run == 0), peak_rss_kib=int(rss.read_text().strip()))
            rows.append(row)
            print(shape, mode, run, row["milliseconds"], flush=True)
    assert len({r["checksum"] for r in rows if r["shape"] == shape}) == 1
summary = []
for shape, mode in sorted({(r["shape"], r["mode"]) for r in rows}):
    group = [r for r in rows if r["shape"] == shape and r["mode"] == mode and not r["warmup"]]
    summary.append(dict(shape=shape, mode=mode,
                        median_ms=statistics.median(r["milliseconds"] for r in group),
                        min_ms=min(r["milliseconds"] for r in group),
                        median_peak_rss_kib=statistics.median(r["peak_rss_kib"] for r in group)))
paths = [Path("benchmark/dynamic-fenwick-tree.cpp"),
         Path("blueberry/data-structure/dynamic-fenwick-tree-2d.hpp"),
         Path("blueberry/data-structure/offline-fenwick-tree-2d.hpp")]
result = dict(generated_at=datetime.now(timezone.utc).isoformat(),
              compiler=subprocess.check_output([compiler, "--version"], text=True).splitlines()[0],
              flags=flags, platform=platform.platform(),
              cpu=next((s.split(":", 1)[1].strip() for s in Path("/proc/cpuinfo").read_text().splitlines()
                        if s.startswith("model name")), "unknown"),
              legacy_revision=baseline, legacy_source_sha256=hashlib.sha256(legacy).hexdigest(),
              source_sha256={p.as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
              seed=1729, repeats=args.repeats, warmups=1, count=args.count,
              timing_scope="Construction, registration for offline, online update/query, destruction; excludes input generation. Legacy pointers explicitly deleted. RSS includes process and inputs.",
              shapes={"bounded": "uniform 20000 x 20000", "repeated": "updates in 100 x 100 within 20000 x 20000",
                      "huge": "uniform 1e9 x 1e9; legacy omitted because initialization is dense"},
              caveat="Offline receives all update coordinates before construction; its functionality differs. No cross-environment leaderboard timing comparisons.",
              summary=summary, runs=rows)
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text(json.dumps(result, indent=2) + "\n")
print(args.output)
