#!/usr/bin/env python3
"""Prepare once, then run serially when other agents' timing jobs are idle."""
import argparse
import hashlib
import itertools
import json
import os
from pathlib import Path
import platform
import random
import statistics
import subprocess
import time

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[4]
PROFILES = {
    "gcc-release": ["g++", "-std=gnu++20", "-O2", "-DNDEBUG"],
    "gcc-assert": ["g++", "-std=gnu++20", "-O2"],
    "clang-release": ["clang++", "-std=gnu++20", "-O2", "-DNDEBUG"],
}
VARIANTS = ["baseline", "candidate", "bit-scan"]
SHAPES = ["path", "star", "balanced", "broom", "random", "permuted"]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=["prepare", "run"])
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--binaries", type=Path, default=Path("/tmp/blueberry-tree-perf-20261017"))
    parser.add_argument("--n", type=int, default=200000)
    parser.add_argument("--q", type=int, default=500000)
    parser.add_argument("--repeats", type=int, default=5)
    parser.add_argument("--profiles", default=",".join(PROFILES))
    parser.add_argument("--variants", default=",".join(VARIANTS))
    parser.add_argument("--kinds", default="doubling,rmq,kth")
    parser.add_argument("--shapes", default=",".join(SHAPES))
    args = parser.parse_args()
    profiles, variants = args.profiles.split(","), args.variants.split(",")
    args.output.mkdir(parents=True, exist_ok=True)
    args.binaries.mkdir(parents=True, exist_ok=True)
    if args.action == "prepare":
        with (args.output / "compile.log").open("w") as log:
            for profile, variant in itertools.product(profiles, variants):
                command = [*PROFILES[profile], "-Wall", "-Wextra", "-Wshadow", "-Werror",
                           "-Wno-range-loop-construct", '-DLCA_HEADER="' + variant + '.hpp"',
                           str(HERE / "benchmark.cpp"), "-I", str(ROOT),
                           "-o", str(args.binaries / (profile + "-" + variant))]
                log.write(json.dumps(command) + "\n"); log.flush()
                subprocess.run(command, stdout=log, stderr=log, check=True)
        print("compiled", len(profiles) * len(variants), "binaries", flush=True)
        return
    cpu = min(os.sched_getaffinity(0))
    os.sched_setaffinity(0, {cpu})
    sources = [HERE / name for name in ["benchmark.cpp", "run.py", "sparse-table-baseline.hpp"]
               + [variant + ".hpp" for variant in variants]]
    env = {
        "baseline_revision": "d6af3e9", "utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "platform": platform.platform(), "cpuinfo": Path("/proc/cpuinfo").read_text().split("\n\n")[0],
        "cpu_affinity": cpu, "profiles": {p: PROFILES[p] for p in profiles},
        "compiler_versions": {c: subprocess.check_output([c, "--version"], text=True)
                              for c in {PROFILES[p][0] for p in profiles}},
        "sources": {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources},
        "n": args.n, "q": args.q, "seed": 20261017, "repeats": args.repeats,
        "warmups": 1, "order": "shuffle configurations each round with seed 917; all serial",
        "scope": "construction excludes tree/query generation; query loop includes checksum; no input parsing/output formatting in timed sections",
        "memory": "peak process RSS includes identical tree/query inputs and runtime; not a table-only allocation measurement",
        "dependency": "SparseTable fixed to d6af3e9 in every variant",
    }
    (args.output / "environment.json").write_text(json.dumps(env, indent=2) + "\n")
    configs = [(p, v, k, s) for p, v, k, s in itertools.product(profiles, variants,
               args.kinds.split(","), args.shapes.split(",")) if not (v == "bit-scan" and k == "rmq")]
    expected, samples = {}, []
    rng = random.Random(917)
    with (args.output / "samples.jsonl").open("w") as log:
        for repeat in range(-1, args.repeats):
            order = configs.copy(); rng.shuffle(order)
            for profile, variant, kind, shape in order:
                binary = args.binaries / (profile + "-" + variant)
                rss_path = args.binaries / "rss.txt"
                command = ["/usr/bin/time", "-f", "%M", "-o", str(rss_path), str(binary),
                           kind, shape, str(args.n), str(args.q), "20261017", "1"]
                started = time.perf_counter()
                proc = subprocess.run(command, text=True, capture_output=True, timeout=120)
                if proc.returncode:
                    raise RuntimeError(json.dumps({"command": command, "code": proc.returncode,
                                                  "stdout": proc.stdout, "stderr": proc.stderr}))
                row = json.loads(proc.stdout)
                key = kind, shape
                if key in expected and expected[key] != row["checksum"]:
                    raise RuntimeError(f"checksum mismatch {key} {variant}")
                expected[key] = row["checksum"]
                row.update(profile=profile, variant=variant, kind=kind, shape=shape,
                           repeat=repeat, warmup=repeat == -1,
                           process_ms=(time.perf_counter() - started) * 1000,
                           peak_rss_kib=int(rss_path.read_text()))
                log.write(json.dumps(row) + "\n"); log.flush()
                if repeat >= 0: samples.append(row)
            print("round", repeat, "matching checksums", len(order), flush=True)
    summary = []
    for profile, variant, kind, shape in configs:
        rows = [r for r in samples if (r["profile"], r["variant"], r["kind"], r["shape"])
                == (profile, variant, kind, shape)]
        summary.append({"profile": profile, "variant": variant, "kind": kind, "shape": shape,
                        **{name: {"median": statistics.median(r[name] for r in rows),
                                  "min": min(r[name] for r in rows)}
                           for name in ["build_ms", "query_ms", "total_ms", "process_ms", "peak_rss_kib"]}})
    (args.output / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")


if __name__ == "__main__":
    main()
