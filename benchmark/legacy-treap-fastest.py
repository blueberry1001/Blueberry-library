#!/usr/bin/env python3
"""Time locally prepared public submissions; keep foreign sources in .build only.

Preparation metadata and generated input files live in .build/legacy-treap.
Run in the same coordinated quiet window as legacy-treap.py.
"""
import hashlib
import json
import pathlib
import platform
import statistics
import subprocess
import time
from datetime import datetime, timezone

ROOT = pathlib.Path(__file__).resolve().parents[1]
BASE = ROOT / ".build/legacy-treap"
OUTPUT = ROOT / "benchmark/results/legacy-treap"
OUTPUT.mkdir(parents=True, exist_ok=True)
prepared = json.loads((BASE / "fastest-prepared.json").read_text())
rows = []
for problem in sorted({row["problem"] for row in prepared["rows"]}):
    variants = [row for row in prepared["rows"] if row["problem"] == problem]
    expected = None
    for repeat in range(-1, 5):
        for variant in variants[::1 if repeat % 2 == 0 else -1]:
            if variant["compile_returncode"]:
                continue
            with (BASE / (problem + ".in")).open("rb") as source:
                begin = time.perf_counter_ns()
                result = subprocess.run([str(ROOT / variant["binary"])], stdin=source,
                                        capture_output=True, timeout=30)
                elapsed = time.perf_counter_ns() - begin
            digest = hashlib.sha256(result.stdout).hexdigest()
            # Public submissions may print padded space-separated fields.
            # These problems judge integer tokens, not byte-identical whitespace.
            token_digest = hashlib.sha256(b"\n".join(result.stdout.split())).hexdigest()
            row = dict(problem=problem, variant=variant["variant"], repeat=repeat,
                       ns=elapsed, returncode=result.returncode, output_sha256=digest,
                       token_sha256=token_digest,
                       stderr=result.stderr.decode(errors="replace"))
            rows.append(row)
            (OUTPUT / "fastest-raw.json").write_text(json.dumps(rows, indent=2) + "\n")
            result.check_returncode()
            if expected is None:
                expected = token_digest
            if token_digest != expected:
                raise RuntimeError(f"Output mismatch: {problem} {variant['variant']}")
summaries = []
for problem, variant in sorted({(r["problem"], r["variant"]) for r in rows}):
    samples = [r["ns"] for r in rows if r["problem"] == problem and
               r["variant"] == variant and r["repeat"] >= 0]
    summaries.append(dict(problem=problem, variant=variant, samples=samples,
                          median_ns=statistics.median(samples), min_ns=min(samples)))
report = dict(generated_at=datetime.now(timezone.utc).isoformat(), prepared=prepared,
              compiler=subprocess.check_output(["g++", "--version"], text=True).splitlines()[0],
              platform=platform.platform(), cpu=subprocess.check_output(["lscpu"], text=True),
              input_sha256={p.name: hashlib.sha256(p.read_bytes()).hexdigest()
                            for p in BASE.glob("*.in")},
              scope="End-to-end process time including each submitted I/O implementation; regular-file stdin; identical local flags and inputs; warm caches after one warmup.",
              load_note="Other agent compilation and official verification paused by coordination.",
              summaries=summaries)
(OUTPUT / "fastest-results.json").write_text(json.dumps(report, indent=2) + "\n")
print(json.dumps(summaries, indent=2))
