#!/usr/bin/env python3
"""Sequential LCT comparison. Run only while other benchmarks are idle."""
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
import tempfile
import time

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[4]
PROFILES = {
    "gcc-release": ["g++", "-std=gnu++20", "-O2", "-DNDEBUG"],
    "gcc-assert": ["g++", "-std=gnu++20", "-O2", "-D_GLIBCXX_ASSERTIONS"],
    "clang-release": ["clang++", "-std=gnu++20", "-O2", "-DNDEBUG"],
}
VARIANTS = ["baseline", "identity-only", "rotation-only", "candidate"]
SHAPES = ["path", "star", "random", "dynamic"]
MONOIDS = ["sum", "affine", "matrix"]


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--output", type=Path, required=True)
    p.add_argument("--profiles", default=",".join(PROFILES))
    p.add_argument("--variants", default=",".join(VARIANTS))
    p.add_argument("--n", type=int, default=8192)
    p.add_argument("--q", type=int, default=60000)
    p.add_argument("--repeats", type=int, default=5)
    p.add_argument("--seed", type=int, default=20260919)
    p.add_argument("--count-ops", action="store_true")
    args = p.parse_args()
    profiles, variants = args.profiles.split(","), args.variants.split(",")
    if not set(profiles) <= PROFILES.keys() or not set(variants) <= set(VARIANTS):
        p.error("unknown profile or variant")
    if "baseline" not in variants or "candidate" not in variants:
        p.error("baseline and candidate are required")
    if args.n < 2 or args.q < 1 or args.repeats < 3:
        p.error("n >= 2, q >= 1 and repeats >= 3 required")
    args.output.mkdir(parents=True, exist_ok=False)
    cpu = min(os.sched_getaffinity(0))
    os.sched_setaffinity(0, {cpu})
    tracked = [HERE / "benchmark.cpp", Path(__file__), *[HERE / (v + ".hpp") for v in variants],
               ROOT / "blueberry/graph/link-cut-tree.hpp", ROOT / "tests/random/link-cut-tree.cpp"]
    hashes = {str(f.relative_to(ROOT)): sha(f) for f in tracked}
    env = {
        "baseline_revision": "e0d8e71", "utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "revision": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
        "platform": platform.platform(), "cpuinfo": Path("/proc/cpuinfo").read_text().split("\n\n")[0],
        "cpu_affinity": cpu, "profiles": {k: PROFILES[k] for k in profiles}, "sources": hashes,
        "compiler_versions": {c: subprocess.check_output([c, "--version"], text=True) for c in {PROFILES[k][0] for k in profiles}},
        "n": args.n, "q": args.q, "seed": args.seed, "repeats": args.repeats,
        "warmups_per_configuration": 1, "ordering": "seed 917 shuffled configurations each round, sequential",
        "scope": "build includes linking initial forest; operations include public API checks and checksum; trace generation is untimed",
        "correctness": "checksum and successful/failed public operations compared across variants; independent random/official tests run separately",
    }
    (args.output / "environment.json").write_text(json.dumps(env, indent=2) + "\n")
    expected = {}
    samples = []
    with tempfile.TemporaryDirectory(prefix="blueberry-lct-") as td:
        temp = Path(td)
        binaries = {}
        with (args.output / "compile.log").open("w") as log:
            for profile, variant in itertools.product(profiles, variants):
                target = temp / (profile + "-" + variant)
                cmd = [*PROFILES[profile], "-Wall", "-Wextra", '-DLCT_HEADER="' + variant + '.hpp"',
                       str(HERE / "benchmark.cpp"), "-o", str(target)]
                log.write(json.dumps(cmd) + "\n"); log.flush()
                subprocess.run(cmd, stdout=log, stderr=log, check=True)
                binaries[profile, variant] = target
            if args.count_ops:
                for variant in variants:
                    target = temp / ("count-" + variant)
                    cmd = [*PROFILES["gcc-release"], "-DLCT_COUNT_OPS", '-DLCT_HEADER="' + variant + '.hpp"',
                           str(HERE / "benchmark.cpp"), "-o", str(target)]
                    log.write(json.dumps(cmd) + "\n"); log.flush()
                    subprocess.run(cmd, stdout=log, stderr=log, check=True)
                    binaries["count", variant] = target

        def execute(profile, variant, monoid, shape):
            cmd = [str(binaries[profile, variant]), monoid, shape, str(args.n), str(args.q), str(args.seed)]
            rss = temp / "rss.txt"
            start = time.perf_counter()
            proc = subprocess.run(["/usr/bin/time", "-f", "%M", "-o", str(rss), *cmd],
                                  capture_output=True, text=True, timeout=180)
            elapsed = (time.perf_counter() - start) * 1000
            if proc.returncode:
                raise RuntimeError(f"failed {cmd}: {proc.returncode} {proc.stderr}")
            row = json.loads(proc.stdout)
            key = monoid, shape
            if key in expected and row["checksum"] != expected[key]:
                raise RuntimeError(f"checksum mismatch: {cmd}: {row['checksum']} != {expected[key]}")
            expected[key] = row["checksum"]
            return {"profile": profile, "variant": variant, "monoid": monoid, "shape": shape,
                    "process_ms": elapsed, "peak_rss_kib": int(rss.read_text()), **row}

        configs = list(itertools.product(profiles, variants, MONOIDS, SHAPES))
        rng = random.Random(917)
        with (args.output / "samples.jsonl").open("w") as log:
            for repeat in range(-1, args.repeats):
                order = configs.copy(); rng.shuffle(order)
                for config in order:
                    row = execute(*config); row.update(repeat=repeat, warmup=repeat == -1)
                    log.write(json.dumps(row) + "\n"); log.flush()
                    if repeat >= 0: samples.append(row)
                print(f"round {repeat}: {len(order)} matching checksums", flush=True)
        if args.count_ops:
            counts = []
            for variant, monoid, shape in itertools.product(variants, MONOIDS, SHAPES):
                row = execute("count", variant, monoid, shape)
                # Instrumented wall times are not performance samples.
                counts.append({k: row[k] for k in ["variant", "monoid", "shape", "checksum", "build_op_calls", "operation_op_calls"]})
            (args.output / "counts.json").write_text(json.dumps(counts, indent=2) + "\n")
    if hashes != {str(f.relative_to(ROOT)): sha(f) for f in tracked}:
        raise RuntimeError("tracked source changed during measurement")
    summary = []
    keys = ["profile", "variant", "monoid", "shape"]
    for config in configs:
        rows = [x for x in samples if tuple(x[k] for k in keys) == config]
        row = dict(zip(keys, config)); row["runs"] = len(rows)
        for k in ["build_ms", "operations_ms", "total_ms", "process_ms", "peak_rss_kib"]:
            values = [x[k] for x in rows]
            row[k] = {"median": statistics.median(values), "min": min(values), "max": max(values)}
        summary.append(row)
    (args.output / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(f"complete: {len(samples)} measured samples; {len(configs)} warmups", flush=True)


if __name__ == "__main__":
    main()
