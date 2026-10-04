#!/usr/bin/env python3
"""Reproduce paired HLD API timings against a Git baseline, without downloads."""
import argparse
import csv
import datetime
import hashlib
import io
import json
import os
from pathlib import Path
import platform
import shlex
import statistics
import subprocess


def capture(command, **kwargs):
    return subprocess.run(command, check=True, capture_output=True, text=True, **kwargs).stdout


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline", default="0dfa0e6c0f1de890feef2af80ae2a3e182ea8f55")
    parser.add_argument("--candidate-header", help="optional saved candidate snapshot to reproduce")
    parser.add_argument("--name", default="paired")
    parser.add_argument("--baseline-only", action="store_true")
    parser.add_argument("--compilers", nargs="+", default=["g++", "clang++"])
    parser.add_argument("--modes", nargs="+", choices=["assert", "release"], default=["assert", "release"])
    parser.add_argument("--n", type=int, default=200000)
    parser.add_argument("--q", type=int, default=200000)
    parser.add_argument("--repeats", type=int, default=7)
    parser.add_argument("--seed", type=int, default=20261004)
    parser.add_argument("--cpu", type=int, default=2)
    parser.add_argument("--prepare-only", action="store_true")
    parser.add_argument("--compile-only", action="store_true")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    if not args.name or Path(args.name).name != args.name:
        parser.error("--name must be one directory name")
    if min(args.n, args.q, args.repeats) <= 0 or args.n < 2:
        parser.error("N >= 2, Q >= 1 and repeats >= 1 required")
    os.chdir(root)
    build = root / ".benchmark/hld" / args.name
    output = root / "benchmark/results/hld" / args.name
    build.mkdir(parents=True, exist_ok=True)
    output.mkdir(parents=True, exist_ok=True)
    if (output / "raw.csv").exists():
        parser.error("output already contains measurements; choose a fresh --name")
    header_path = "blueberry/graph/heavy-light-decomposition.hpp"
    baseline = subprocess.run(["git", "show", f"{args.baseline}:{header_path}"],
                              check=True, capture_output=True).stdout
    (build / "baseline.hpp").write_bytes(baseline)
    candidate = Path(args.candidate_header) if args.candidate_header else root / header_path
    candidate_source = candidate.read_bytes()
    (build / "candidate.hpp").write_bytes(candidate_source)
    (output / "candidate.hpp").write_bytes(candidate_source)
    os.sched_setaffinity(0, {args.cpu})
    environment = {
        "created_at_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "platform": platform.platform(), "uname": list(platform.uname()),
        "cpu_info": capture(["lscpu"]), "affinity": sorted(os.sched_getaffinity(0)),
        "baseline_revision": capture(["git", "rev-parse", args.baseline]).strip(),
        "working_revision": capture(["git", "rev-parse", "HEAD"]).strip(),
        "git_status": capture(["git", "status", "--short"]),
        "baseline_header_sha256": sha256(build / "baseline.hpp"),
        "candidate_header_sha256": sha256(build / "candidate.hpp"),
        "harness_sha256": sha256(root / "benchmark/hld.cpp"),
        "runner_sha256": sha256(Path(__file__)),
        "arguments": vars(args), "commands": [],
        "method": [
            "Construction timed separately; tree creation, layout inspection and destruction excluded.",
            "Queries run on prebuilt structures; generated inputs and I/O excluded.",
            "Both variants share the same immutable graph/query objects; variant order alternates per repeat.",
            "One warm-up per operation and variant precedes measured repeats.",
            "Checksum and callback-count agreement checked outside timing; unsigned rolling callback checksum inside.",
            "Ten percent of queries have identical endpoints, ten percent ancestor endpoints (1..32 parent steps, both directions), remainder uniform pairs; subtree queries mix vertex and edge modes.",
            "Random-parent trees attach vertex v to a uniform vertex in [0,v); not uniform labelled trees.",
            "Checksums deliberately require identical Euler layout and ordered callback segmentation.",
            "Timings are microbenchmarks, not Library Checker or application end-to-end measurements.",
        ],
    }
    environment_path = output / "environment.json"
    environment_path.write_text(json.dumps(environment, indent=2) + "\n")
    if args.prepare_only:
        print(f"Prepared baseline {environment['baseline_header_sha256']} in {build}")
        return
    rows = []
    for compiler in args.compilers:
        version = capture([compiler, "--version"]).splitlines()[0]
        for mode in args.modes:
            flags = ["-std=gnu++20", "-O2", "-Wall", "-Wextra", "-Werror"]
            if mode == "release":
                flags.append("-DNDEBUG")
            binary = build / f"{Path(compiler).name}-{mode}"
            command = [compiler, *flags,
                       f'-DHLD_BASELINE_HEADER="{build / "baseline.hpp"}"',
                       f'-DHLD_CANDIDATE_HEADER="{build / "candidate.hpp"}"',
                       str(root / "benchmark/hld.cpp"), "-o", str(binary)]
            print(f"Compiling {compiler} {mode}", flush=True)
            completed = subprocess.run(command, capture_output=True, text=True)
            (output / f"{Path(compiler).name}-{mode}-compile.log").write_text(completed.stdout + completed.stderr)
            completed.check_returncode()
            run_command = [str(binary), str(args.n), str(args.q), str(args.repeats),
                           str(args.seed), str(int(args.baseline_only))]
            environment["commands"].append({"compiler": version, "mode": mode,
                                             "compile": shlex.join(command), "run": shlex.join(run_command)})
            environment_path.write_text(json.dumps(environment, indent=2) + "\n")
            if args.compile_only:
                continue
            completed = subprocess.run(run_command, capture_output=True, text=True)
            (output / f"{Path(compiler).name}-{mode}-raw.csv").write_text(completed.stdout)
            (output / f"{Path(compiler).name}-{mode}-stderr.log").write_text(completed.stderr)
            completed.check_returncode()
            for row in csv.DictReader(io.StringIO(completed.stdout)):
                rows.append({"compiler": compiler, "mode": mode, **row})
            print(f"Completed {compiler} {mode}", flush=True)
    if args.compile_only:
        print(f"Compiled without measurements: {output}")
        return
    with (output / "raw.csv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]), lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)
    groups = {}
    for row in rows:
        key = tuple(row[column] for column in ["compiler", "mode", "shape", "variant", "operation"])
        groups.setdefault(key, []).append(float(row["ms"]))
    summary = []
    for key, samples in groups.items():
        compiler, mode, shape, variant, operation = key
        baseline_key = compiler, mode, shape, "baseline", operation
        median = statistics.median(samples)
        summary.append({"compiler": compiler, "mode": mode, "shape": shape,
                        "variant": variant, "operation": operation, "repeats": len(samples),
                        "n": args.n, "q": args.q, "median_ms": median, "min_ms": min(samples),
                        "max_ms": max(samples),
                        "candidate_over_baseline": median / statistics.median(groups[baseline_key])
                            if variant == "candidate" else ""})
    with (output / "summary.csv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(summary[0]), lineterminator="\n")
        writer.writeheader()
        writer.writerows(summary)
    print(output)


if __name__ == "__main__":
    main()
