#!/usr/bin/env python3
"""Preserve paired sum-tree phase timings, compiler variants and raw measurements.

Run inside Linux/WSL. Results are never overwritten. Every fast variant and
baseline receives the same deterministic text fixture and the same arithmetic
contract (uint64_t sum modulo 2^64, online add/set/prod).
"""

import argparse
import hashlib
import json
import os
import platform
import re
import statistics
import subprocess
import tempfile
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
CONFIGURATIONS = {
    "portable-o2-release": ["-O2", "-DNDEBUG"],
    "native-o2-release": ["-O2", "-march=native", "-DNDEBUG"],
    "portable-o3-release": ["-O3", "-DNDEBUG"],
    "native-o3-release": ["-O3", "-march=native", "-DNDEBUG"],
    "portable-o2-debug": ["-O2"],
}
CASES = [
    (4096, 200000, "random"),
    (65536, 200000, "random"),
    (262144, 50000, "queries"),
    (1048576, 200000, "random"),
    (65536, 200000, "short"),
    (65536, 200000, "updates"),
    (65536, 200000, "point-add"),
]
VARIANTS = ["fast-asm", "fast-intrinsics", "fast-scalar", "acl-fenwick", "acl-segtree", "blueberry"]


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def source_hashes(acl):
    paths = [HERE / "benchmark.cpp", HERE / "run.py",
             ROOT / "blueberry/extra-fast/segment-tree.hpp",
             ROOT / "blueberry/data-structure/segment-tree.hpp"]
    paths += sorted(p for p in (acl / "atcoder").glob("*") if p.is_file())
    return {str(p.relative_to(ROOT)) if p.is_relative_to(ROOT) else str(p): sha256(p)
            for p in paths}


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")


def git_metadata(*arguments):
    # Windows-created worktrees contain an absolute Windows gitdir path. WSL's
    # git needs that path translated; do not edit the shared worktree metadata.
    command = ["git"]
    dotgit = ROOT / ".git"
    if os.name != "nt" and dotgit.is_file():
        target = dotgit.read_text().strip().removeprefix("gitdir: ")
        if re.match(r"^[A-Za-z]:[/\\]", target):
            target = "/mnt/" + target[0].lower() + target[2:].replace("\\", "/")
            command += ["--git-dir", target, "--work-tree", str(ROOT)]
    return subprocess.check_output([*command, *arguments], cwd=ROOT, text=True).strip()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--acl-root", type=Path, default=ROOT / ".deps/ac-library")
    parser.add_argument("--compiler", action="append", help="Repeat for multiple compilers (default g++, clang++)")
    parser.add_argument("--configuration", action="append", choices=CONFIGURATIONS)
    parser.add_argument("--repeats", type=int, default=5)
    parser.add_argument("--control-header", type=Path,
                        help="Also compile a current-header control using its default intrinsics (same CPU/input/flags)")
    parser.add_argument("--quick", action="store_true", help="Only first two workloads; all selected flags/variants still measured")
    args = parser.parse_args(argv)
    if args.repeats < 1:
        parser.error("--repeats must be positive")
    acl = args.acl_root.resolve()
    if not (acl / "atcoder/segtree.hpp").exists():
        parser.error("--acl-root must contain atcoder/segtree.hpp")
    output = args.output.resolve()
    if output.exists() and any(output.iterdir()):
        parser.error("refusing to overwrite a nonempty output directory")
    output.mkdir(parents=True, exist_ok=True)
    compilers = args.compiler or ["g++", "clang++"]
    configurations = args.configuration or list(CONFIGURATIONS)
    cases = CASES[:2] if args.quick else CASES
    baseline_header = args.control_header.resolve() if args.control_header else None
    if baseline_header and not baseline_header.is_file():
        parser.error("--control-header must be an existing header")
    variants = VARIANTS + (["fast-control-intrinsics"] if baseline_header else [])
    initial_hashes = source_hashes(acl)
    environment = {
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "platform": platform.platform(), "python": platform.python_version(),
        "cpu_affinity": sorted(os.sched_getaffinity(0)) if hasattr(os, "sched_getaffinity") else None,
        "cpuinfo": Path("/proc/cpuinfo").read_text().split("\n\n")[0]
        if Path("/proc/cpuinfo").exists() else None,
        "revision": git_metadata("rev-parse", "HEAD"),
        "working_tree": git_metadata("status", "--short"),
        "source_sha256": initial_hashes, "compilers": {},
        "configurations": configurations, "cases": cases, "runs": args.repeats,
        "seed": 20261019,
        "arithmetic": "uint64_t sum modulo 2^64; online add, assignment and half-open range sum",
        "operations": "50% updates normally, 90% in updates, 10% in queries; point-add uses add only, other cases split add/set equally",
        "short_range": "random left endpoint, length 0..32 clipped to N",
        "order": "Rotate candidate order by run and workload; fresh process per sample, no warmup",
        "timing": "generation excluded; parse/from_chars, build, operations including result storage, format/to_chars separately; end_to_end is their sum, no OS file I/O",
        "memory": "Linux /proc/self/status VmHWM peak RSS KiB (new executable address space, excludes inherited parent peak); memory samples contain executable, input values and live tree only, no serialized fixture or operations; timing samples include all buffers",
        "fenwick_build": "ACL public add loop O(N log N); no private state access or changed constructor contract",
        "dispatch": "asm/intrinsics use the same runtime AVX2 check; scalar binary defines FORCE_SCALAR",
        "control_header": {"path": str(baseline_header), "sha256": sha256(baseline_header)}
        if baseline_header else None,
    }
    write_json(output / "environment.json", environment)
    for relative in ["blueberry/extra-fast/segment-tree.hpp",
                     "docs/development/measurements/extra-fast/benchmark.cpp",
                     "docs/development/measurements/extra-fast/run.py"]:
        (output / (Path(relative).name + ".snapshot")).write_bytes((ROOT / relative).read_bytes())
    if baseline_header:
        (output / "control-segment-tree.hpp.snapshot").write_bytes(baseline_header.read_bytes())
    rows, failures = [], []
    with tempfile.TemporaryDirectory(prefix="extra-fast-bench-") as temporary:
        with (output / "compile.log").open("w", encoding="utf-8") as compile_log:
            with (output / "samples.jsonl").open("w", encoding="utf-8") as samples:
                for compiler in compilers:
                    try:
                        environment["compilers"][compiler] = subprocess.check_output([compiler, "--version"], text=True)
                    except (OSError, subprocess.CalledProcessError) as error:
                        failures.append({"compiler": compiler, "error": str(error)})
                        continue
                    write_json(output / "environment.json", environment)
                    for configuration in configurations:
                        key = f"{Path(compiler).name}-{configuration}"
                        binaries = {}
                        modes = [
                            ("asm", ["-DBLUEBERRY_EXTRA_FAST_USE_ASM"]),
                            ("intrinsics", []),
                            ("scalar", ["-DBLUEBERRY_EXTRA_FAST_FORCE_SCALAR"]),
                        ]
                        if baseline_header:
                            modes.append(("control-intrinsics", [
                                f'-DBLUEBERRY_EXTRA_FAST_HEADER="{baseline_header}"']))
                        for mode, extra in modes:
                            binary = Path(temporary) / f"{key}-{mode}"
                            command = [compiler, "-std=gnu++20", *CONFIGURATIONS[configuration],
                                       *extra, "-I", str(ROOT), "-isystem", str(acl),
                                       str(HERE / "benchmark.cpp"), "-o", str(binary)]
                            compiled = subprocess.run(command, capture_output=True, text=True)
                            diagnostics = compiled.stdout + compiled.stderr
                            compile_log.write(json.dumps(command) + "\n" + diagnostics)
                            if diagnostics and not diagnostics.endswith("\n"):
                                compile_log.write("\n")
                            compile_log.flush()
                            if compiled.returncode:
                                failures.append({"command": command, "returncode": compiled.returncode,
                                                 "stdout": compiled.stdout, "stderr": compiled.stderr})
                            else:
                                binaries[mode] = binary
                        if len(binaries) != len(modes):
                            continue
                        for case_index, (n, q, distribution) in enumerate(cases):
                            for run in range(args.repeats):
                                shift = (run + case_index) % len(variants)
                                for variant in variants[shift:] + variants[:shift]:
                                    mode = variant.removeprefix("fast-") if variant.startswith("fast-") else "asm"
                                    implementation = "fast" if variant.startswith("fast-") else variant
                                    # Separate memory-only samples avoid input/operation buffers hiding structure size.
                                    phases = ["timing", "memory"] if case_index == next(i for i, c in enumerate(cases) if c[0] == n) else ["timing"]
                                    for phase in phases:
                                        command = [str(binaries[mode]), implementation, str(n), str(q), distribution, phase]
                                        result = subprocess.run(command, capture_output=True, text=True, timeout=60)
                                        if result.returncode:
                                            failures.append({"command": command, "returncode": result.returncode,
                                                             "stdout": result.stdout, "stderr": result.stderr})
                                            continue
                                        row = json.loads(result.stdout)
                                        row.update(compiler=compiler, configuration=configuration, variant=variant,
                                                   n=n, q=q, distribution=distribution, run=run, phase=phase)
                                        rows.append(row)
                                        samples.write(json.dumps(row) + "\n")
                                        samples.flush()
                            selected = [r for r in rows if (r["compiler"], r["configuration"], r["n"], r["q"], r["distribution"], r["phase"])
                                        == (compiler, configuration, n, q, distribution, "timing")]
                            for field in ["checksum", "input_hash", "input_bytes", "output_bytes"]:
                                if len({r[field] for r in selected}) != 1:
                                    failures.append({"configuration": key, "n": n, "q": q,
                                                     "distribution": distribution, "error": f"{field} mismatch"})
                            print(f"{key} N={n} Q={q} {distribution}: {len(selected)} timing samples", flush=True)
    if initial_hashes != source_hashes(acl):
        failures.append({"error": "Source changed during measurement", "final_hashes": source_hashes(acl)})
    summary = []
    group_fields = ["compiler", "configuration", "variant", "n", "q", "distribution", "phase"]
    groups = sorted({tuple(row[field] for field in group_fields) for row in rows})
    for group in groups:
        selected = [r for r in rows if tuple(r[field] for field in group_fields) == group]
        aggregate = dict(zip(group_fields, group))
        aggregate["samples"] = len(selected)
        fields = ["peak_rss_kib"] if group[-1] == "memory" else [
            "parse_ms", "build_ms", "operations_ms", "format_ms", "end_to_end_ms", "peak_rss_kib"]
        for field in fields:
            aggregate[f"median_{field}"] = statistics.median(r[field] for r in selected)
            aggregate[f"min_{field}"] = min(r[field] for r in selected)
        summary.append(aggregate)
    write_json(output / "summary.json", summary)
    write_json(output / "failures.json", failures)
    print(f"Saved {len(rows)} raw samples, {len(failures)} failures to {output}", flush=True)
    return int(bool(failures))


if __name__ == "__main__":
    raise SystemExit(main())
