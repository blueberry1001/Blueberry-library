#!/usr/bin/env python3
"""Prepare furthest-pair comparisons; run timings only in a coordinated quiet window."""

import argparse
import hashlib
import json
import pathlib
import platform
import shutil
import statistics
import subprocess
import time
from datetime import datetime, timezone


ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = ROOT / ".build/furthest-pair"
OUT = ROOT / "benchmark/results/furthest-pair"
SOURCE = ROOT / "benchmark/furthest-pair.cpp"
SOURCES = [SOURCE, ROOT / "blueberry/geometry/furthest-pair.hpp",
           ROOT / "blueberry/geometry/convex-hull.hpp", ROOT / "blueberry/utility/fast-io.hpp"]
WORKLOADS = ["random", "duplicates", "all-same", "collinear", "parabola", "sorted", "wide", "tiny"]
VARIANTS = ["blueberry", "indices", "records", "fast-cases"]
IO_WORKLOADS = ["random", "tiny"]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write(name, value):
    (OUT / name).write_text(json.dumps(value, indent=2) + "\n")


def prepare(args):
    BUILD.mkdir(parents=True, exist_ok=True)
    OUT.mkdir(parents=True, exist_ok=True)
    entries = []
    for compiler in args.compilers:
        for mode in ["release", "assert"]:
            binary = BUILD / (pathlib.Path(compiler).name + "-" + mode)
            flags = ["-std=gnu++20", "-O2", "-Wall", "-Wextra", "-I", str(ROOT)]
            if mode == "release":
                flags.append("-DNDEBUG")
            command = [compiler, *flags, str(SOURCE), "-o", str(binary)]
            result = subprocess.run(command, capture_output=True, text=True)
            entries.append(dict(compiler=compiler, mode=mode, command=command,
                                compiler_version=subprocess.check_output([compiler, "--version"], text=True),
                                binary=str(binary.relative_to(ROOT)), returncode=result.returncode,
                                stdout=result.stdout, stderr=result.stderr,
                                binary_sha256=sha(binary) if result.returncode == 0 else None))
            write("compile.json", entries)
            result.check_returncode()
            check = subprocess.run([str(binary), "check"], capture_output=True, text=True)
            entries[-1]["correctness"] = dict(returncode=check.returncode, stdout=check.stdout, stderr=check.stderr)
            write("compile.json", entries)
            check.check_returncode()
    prepared = dict(created_at=datetime.now(timezone.utc).isoformat(), entries=entries,
                    source_sha256={str(path.relative_to(ROOT)): sha(path) for path in SOURCES},
                    git_head=subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
                    input_size=args.size)
    for workload in [*IO_WORKLOADS, "parabola", "all-same"]:
        with (BUILD / (workload + ".in")).open("wb") as output:
            subprocess.run([str(ROOT / entries[0]["binary"]), "generate", workload, str(args.size)],
                           stdout=output, check=True)
    prepared["input_sha256"] = {workload: sha(BUILD / (workload + ".in"))
                               for workload in [*IO_WORKLOADS, "parabola", "all-same"]}
    write("prepared.json", prepared)
    print("Prepared four binaries, checked each against all-pairs distance, and generated inputs. No timings collected.")


def checked_prepared():
    prepared = json.loads((OUT / "prepared.json").read_text())
    for relative, digest in prepared["source_sha256"].items():
        if sha(ROOT / relative) != digest:
            raise RuntimeError(f"Source changed since preparation: {relative}")
    for entry in prepared["entries"]:
        if sha(ROOT / entry["binary"]) != entry["binary_sha256"]:
            raise RuntimeError("Binary changed since preparation")
    for workload, digest in prepared["input_sha256"].items():
        if sha(BUILD / (workload + ".in")) != digest:
            raise RuntimeError("Input changed since preparation")
    return prepared


def summary(samples):
    fields = [key for key, value in samples[0].items()
              if isinstance(value, (int, float)) and key.endswith("_ns")]
    return {key: dict(median=statistics.median(sample[key] for sample in samples),
                      min=min(sample[key] for sample in samples)) for key in fields}


def quiet_snapshot():
    snapshot = subprocess.check_output(["ps", "-eo", "pid,pcpu,stat,comm"], text=True)
    (OUT / "quiet-processes.txt").write_text(snapshot)
    for line in snapshot.splitlines():
        columns = line.split()
        if len(columns) >= 4 and columns[3] in {"cc1plus", "clang", "clang++", "clang-19", "g++", "g++-14"}:
            if "T" not in columns[2] and "Z" not in columns[2]:
                raise RuntimeError("Compiler active; coordinate another quiet window")


def small_parent_command(binary, *args):
    # A child of this small shell does not inherit Python's high RSS floor.
    return ["/bin/sh", "-c", '"$@"; result=$?; exit "$result"',
            "furthest-benchmark", str(binary), *args]


def run(args):
    prepared = checked_prepared()
    quiet_snapshot()
    rows = []
    expected = {}
    n = prepared["input_size"]
    for ei, entry in enumerate(prepared["entries"]):
        for wi, workload in enumerate(WORKLOADS):
            offset = (wi + ei) % len(VARIANTS)
            for variant in VARIANTS[offset:] + VARIANTS[:offset]:
                command = [str(ROOT / entry["binary"]), variant, workload, str(n), str(args.repeats)]
                result = subprocess.run(command, capture_output=True, text=True, timeout=120, check=True)
                data = json.loads(result.stdout)
                identities = {sample["objective_digest"] for sample in data["samples"]}
                if len(identities) != 1:
                    raise RuntimeError("Objective varied between repetitions")
                identity = (data["input_digest"], data["cases"], identities.pop())
                if identity != expected.setdefault(workload, identity):
                    raise RuntimeError(f"Candidate mismatch: {workload} {variant}")
                rows.append(dict(compiler=entry["compiler"], mode=entry["mode"], variant=variant,
                                 workload=workload, size_parameter=n, **data,
                                 summary=summary(data["samples"])))
                write("kernel.json", rows)
    io_rows = []
    io_expected = {}
    for entry in prepared["entries"]:
        for workload in IO_WORKLOADS:
            for repeat in range(-1, args.repeats):
                variants = VARIANTS if repeat % 2 else VARIANTS[::-1]
                for variant in variants:
                    for kind in ["fast", "iostream"]:
                        command = small_parent_command(ROOT / entry["binary"], "io", variant, kind)
                        with (BUILD / (workload + ".in")).open("rb") as source:
                            begin = time.perf_counter_ns()
                            result = subprocess.run(command, stdin=source, capture_output=True, timeout=120, check=True)
                            elapsed = time.perf_counter_ns() - begin
                        data = json.loads(result.stderr)
                        identity = (data["cases"], data["objective_digest"])
                        if identity != io_expected.setdefault(workload, identity):
                            raise RuntimeError(f"I/O objective mismatch: {workload} {variant} {kind}")
                        if len(result.stdout.splitlines()) != data["cases"]:
                            raise RuntimeError("Wrong number of output pairs")
                        if repeat >= 0:
                            io_rows.append(dict(compiler=entry["compiler"], mode=entry["mode"], variant=variant,
                                                workload=workload, io=kind, repeat=repeat, size_parameter=n,
                                                end_to_end_ns=elapsed, output_sha256=hashlib.sha256(result.stdout).hexdigest(),
                                                **data))
                            write("io.json", io_rows)
    io_summary = []
    keys = ["compiler", "mode", "variant", "workload", "io"]
    groups = sorted({tuple(row[key] for key in keys) for row in io_rows})
    for group in groups:
        selected = [row for row in io_rows if tuple(row[key] for key in keys) == group]
        io_summary.append(dict(zip(keys, group), summary=summary(selected),
                               peak_rss_kib=max(row["rss_kib"] for row in selected)))
    report = dict(created_at=datetime.now(timezone.utc).isoformat(), prepared=prepared,
                  platform=platform.platform(), cpu=subprocess.check_output(["lscpu"], text=True),
                  repeats=args.repeats, warmups=1, io_summary=io_summary,
                  conditions="Coordinated quiet window; shared-host activity remains possible. "
                             "Full calls accept immutable input by reference. Separate staged calls model "
                             "hull construction, calipers and coordinate-to-index recovery. I/O end-to-end includes "
                             "small-shell/process startup, output pipe and objective validation; phase timers exclude validation. "
                             "All input cases parsed before solving to separate I/O phases; this uses more memory than a streaming verifier. "
                             "RSS is Linux getrusage KiB measured beneath a small parent, not allocated capacity. No -march=native.")
    write("results.json", report)
    print(f"Completed {len(rows)} kernel configurations and {len(io_rows)} measured I/O runs.")


def memory(args):
    prepared = checked_prepared()
    rows = []
    for entry in prepared["entries"]:
        for workload in ["random", "parabola", "all-same"]:
            for variant in VARIANTS:
                for repeat in range(args.repeats):
                    command = small_parent_command(ROOT / entry["binary"], "io", variant, "fast")
                    with (BUILD / (workload + ".in")).open("rb") as source:
                        result = subprocess.run(command, stdin=source, stdout=subprocess.DEVNULL,
                                                stderr=subprocess.PIPE, text=True, timeout=120, check=True)
                    data = json.loads(result.stderr)
                    rows.append(dict(compiler=entry["compiler"], mode=entry["mode"], variant=variant,
                                     workload=workload, repeat=repeat, rss_kib=data["rss_kib"],
                                     cases=data["cases"], objective_digest=data["objective_digest"]))
                    write("memory.json", dict(conditions="Separate memory-only runs beneath a small shell; "
                                             "getrusage(RUSAGE_SELF).ru_maxrss, Linux KiB. FastIO; output /dev/null. "
                                             "No timing conclusions from these runs.", rows=rows))
    print(f"Completed {len(rows)} memory-only runs.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=["prepare", "run", "memory"])
    parser.add_argument("--compilers", nargs="+", default=["g++", "clang++"])
    parser.add_argument("--size", type=int, default=200000)
    parser.add_argument("--repeats", type=int, default=5)
    args = parser.parse_args()
    if args.size < 2 or args.size > 10000000 or args.repeats < 3:
        parser.error("size must be in [2, 10000000], repeats >= 3")
    {"prepare": prepare, "run": run, "memory": memory}[args.action](args)


if __name__ == "__main__":
    main()
