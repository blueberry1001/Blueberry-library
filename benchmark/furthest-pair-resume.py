#!/usr/bin/env python3
"""Immutable resumed furthest-pair comparisons; source env.sh before running."""

import argparse
import hashlib
import json
import os
import pathlib
import platform
import shutil
import statistics
import subprocess
import time
from datetime import datetime, timezone

ROOT = pathlib.Path(__file__).resolve().parents[1]
BASE = ROOT / "benchmark/results/furthest-pair-resume"
SOURCE = ROOT / "benchmark/furthest-pair-resume.cpp"
HEADERS = ["blueberry/geometry/furthest-pair.hpp", "blueberry/geometry/convex-hull.hpp",
           "blueberry/utility/fast-io.hpp"]
VARIANTS = ["blueberry", "indices", "records", "fast-cases"]
LARGE = ["random", "duplicates", "all-same", "collinear", "parabola", "sorted",
         "reversed", "wide", "late-different"]
TINY = ["tiny"]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def now():
    return datetime.now(timezone.utc).isoformat()


def write(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n")


def command_result(command, *, input_path=None, timeout=120, binary=False):
    begin = time.perf_counter_ns()
    try:
        with input_path.open("rb") if input_path else open(os.devnull, "rb") as stream:
            result = subprocess.run(command, stdin=stream, capture_output=True,
                                    text=not binary, timeout=timeout)
        return dict(command=command, returncode=result.returncode,
                    stdout=result.stdout.decode() if binary else result.stdout,
                    stderr=result.stderr.decode() if binary else result.stderr,
                    process_ns=time.perf_counter_ns() - begin)
    except subprocess.TimeoutExpired as error:
        return dict(command=command, returncode=None, timeout=timeout,
                    stdout=(error.stdout or b"").decode(), stderr=(error.stderr or b"").decode(),
                    process_ns=time.perf_counter_ns() - begin)


def execute(command, out, *, input_path=None, json_stream="stdout", required=()):
    record = command_result(command, input_path=input_path)
    try:
        if record["returncode"] != 0:
            raise RuntimeError("nonzero exit or timeout")
        data = json.loads(record[json_stream])
        if not isinstance(data, dict) or not all(key in data for key in required):
            raise RuntimeError("missing required JSON fields")
        for key, value in data.items():
            if key.endswith("_ns") and value is not None and (not isinstance(value, int) or value < 0):
                raise RuntimeError("invalid duration")
        return record, data
    except Exception as error:
        record["failure"] = str(error)
        write(out / ("failure-" + str(time.time_ns()) + ".json"), record)
        raise


def small_parent(binary, *args):
    return ["/bin/sh", "-c", '"$@"; result=$?; exit "$result"',
            "furthest-resume", str(binary), *args]


def prepare(args):
    out = BASE / args.experiment
    if out.exists():
        raise RuntimeError(f"Refusing to replace existing experiment: {out}")
    out.mkdir(parents=True)
    snapshot = out / "snapshot"
    snapshot.mkdir()
    shutil.copy2(SOURCE, snapshot / SOURCE.name)
    shutil.copy2(pathlib.Path(__file__), out / "runner.py")
    for name in HEADERS:
        target = snapshot / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(ROOT / name, target)
    build = ROOT / ".build/furthest-pair-resume" / args.experiment
    if build.exists():
        raise RuntimeError("Refusing to reuse a binary directory")
    build.mkdir(parents=True)
    prepared = dict(created_at=now(), git_head=subprocess.check_output(
        ["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
        source_sha256=sha(SOURCE), runner_sha256=sha(pathlib.Path(__file__)),
        snapshot_sha256={str(p.relative_to(out)): sha(p) for p in snapshot.rglob("*") if p.is_file()},
        production_header_sha256=sha(ROOT / HEADERS[0]),
        sizes=args.sizes, tiny_size=args.tiny_size, entries=[], inputs={})
    for compiler in args.compilers:
        for mode in ["release", "assert"]:
            for diagnostic in [False, True] if compiler == args.compilers[0] else [False]:
                name = pathlib.Path(compiler).name + "-" + mode + ("-allocation" if diagnostic else "")
                binary = build / name
                flags = ["-std=gnu++20", "-O2", "-Wall", "-Wextra", "-I", str(snapshot)]
                if mode == "release":
                    flags.append("-DNDEBUG")
                if diagnostic:
                    flags += ["-DFP_ALLOCATION_DIAGNOSTIC", "-Wno-mismatched-new-delete"]
                command = [compiler, *flags, str(snapshot / SOURCE.name), "-o", str(binary)]
                record = command_result(command)
                entry = dict(compiler=compiler, mode=mode, diagnostic=diagnostic,
                             compiler_version=subprocess.check_output([compiler, "--version"], text=True),
                             binary=str(binary.relative_to(ROOT)), compile=record)
                prepared["entries"].append(entry)
                write(out / "compile.json", prepared["entries"])
                if record["returncode"] != 0:
                    raise RuntimeError("Compilation failed; compile.json preserved")
                entry["binary_sha256"] = sha(binary)
                entry["correctness"] = command_result([str(binary), "check"])
                if mode == "assert":
                    entry["invalid_singleton"] = [command_result(
                        [str(binary), "invalid-singleton", variant]) for variant in VARIANTS]
                write(out / "compile.json", prepared["entries"])
                if entry["correctness"]["returncode"] != 0 or any(
                        row["returncode"] == 0 for row in entry.get("invalid_singleton", [])):
                    raise RuntimeError("Correctness/negative-contract check failed")
    input_binary = ROOT / prepared["entries"][0]["binary"]
    for workload in ["random", "tiny", "parabola", "all-same"]:
        path = build / (workload + ".in")
        with path.open("wb") as stream:
            result = subprocess.run([str(input_binary), "generate", workload, str(max(args.sizes))],
                                    stdout=stream, stderr=subprocess.PIPE)
        if result.returncode:
            write(out / "input-generation-failure.json", dict(workload=workload, stderr=result.stderr.decode()))
            raise RuntimeError("Input generation failed")
        prepared["inputs"][workload] = dict(path=str(path.relative_to(ROOT)), sha256=sha(path), bytes=path.stat().st_size)
    # Cross-compiler/candidate checks on the sample schema and each workload.
    checks, identities = [], {}
    for entry in prepared["entries"]:
        for workload in LARGE + TINY:
            for variant in VARIANTS:
                command = small_parent(ROOT / entry["binary"], "sample", variant, workload, "64")
                record, data = execute(command, out, required=("full_ns", "construct_ns", "calipers_ns",
                    "recovery_ns", "input_digest", "objective_digest", "cases", "points", "hull_sum", "hull_min", "hull_max"))
                checks.append(dict(compiler=entry["compiler"], mode=entry["mode"], diagnostic=entry["diagnostic"],
                                   workload=workload, variant=variant, data=data, command=command))
                identity = tuple(data[k] for k in ["input_digest", "objective_digest", "cases", "points", "hull_sum"])
                if identity != identities.setdefault(workload, identity):
                    write(out / "small-check-failure.json", dict(rows=checks, rejected=record))
                    raise RuntimeError("Small cross-candidate identity mismatch")
    write(out / "small-checks.json", checks)
    prepared["small_checks_sha256"] = sha(out / "small-checks.json")
    write(out / "prepared.json", prepared)
    print(f"Prepared {len(prepared['entries'])} binaries and {len(checks)} small profiles; no benchmark timings collected.")


def checked(args):
    out = BASE / args.experiment
    prepared = json.loads((out / "prepared.json").read_text())
    for name, digest in prepared["snapshot_sha256"].items():
        if sha(out / name) != digest:
            raise RuntimeError(f"Snapshot changed: {name}")
    if sha(out / "runner.py") != prepared["runner_sha256"] or sha(pathlib.Path(__file__)) != prepared["runner_sha256"]:
        raise RuntimeError("Runner changed after preparation")
    if sha(ROOT / HEADERS[0]) != prepared["production_header_sha256"]:
        raise RuntimeError("Production header changed; use a separately prepared final-code experiment")
    if sha(out / "small-checks.json") != prepared["small_checks_sha256"]:
        raise RuntimeError("Small-check gate changed")
    for entry in prepared["entries"]:
        if sha(ROOT / entry["binary"]) != entry["binary_sha256"]:
            raise RuntimeError("Binary changed")
    for entry in prepared["inputs"].values():
        if sha(ROOT / entry["path"]) != entry["sha256"]:
            raise RuntimeError("Input changed")
    return out, prepared


def environment(out):
    processes = subprocess.check_output(["ps", "-eo", "pid,pcpu,stat,comm"], text=True)
    (out / "quiet-processes.txt").write_text(processes)
    for line in processes.splitlines():
        fields = line.split()
        if len(fields) >= 4 and fields[3] in {"cc1plus", "clang", "clang++", "clang-19", "g++", "g++-14"}:
            if "T" not in fields[2] and "Z" not in fields[2]:
                raise RuntimeError("Active compiler; coordinate a quiet window")
    limits = {}
    for name in ["cpu.max", "cpuset.cpus.effective", "memory.max"]:
        path = pathlib.Path("/sys/fs/cgroup") / name
        limits[name] = path.read_text().strip() if path.exists() else None
    return dict(platform=platform.platform(), cpu=subprocess.check_output(["lscpu"], text=True),
                cgroup=limits, affinity=sorted(os.sched_getaffinity(0)),
                caveat="Coordinated quiet window, unpinned shared host; unrelated host activity remains possible.")


def stats(rows):
    keys = [key for key in rows[0] if key.endswith("_ns") and rows[0][key] is not None]
    return {key: dict(median=statistics.median(row[key] for row in rows),
                      min=min(row[key] for row in rows), max=max(row[key] for row in rows)) for key in keys}


def run(args):
    out, prepared = checked(args)
    result = out / "measurement"
    if result.exists():
        raise RuntimeError("Measurement directory already exists; preserve it")
    result.mkdir()
    metadata = dict(started_at=now(), environment=environment(result), prepared_sha256=sha(out / "prepared.json"),
                    repeats=args.repeats, warmups=1, conditions="Full public calls use the exact snapshot. Stage calls are an independent int-index caliper decomposition, not instrumented production calls or additive elapsed totals. All generation/objective validation/hull metadata outside kernel timers; input immutable by reference. No march=native; allocation diagnostics separate.")
    write(result / "metadata.json", metadata)
    expected, rows = {}, []
    configs = [(workload, n) for n in prepared["sizes"] for workload in LARGE]
    configs += [(workload, prepared["tiny_size"]) for workload in TINY]
    timing = [entry for entry in prepared["entries"] if not entry["diagnostic"]]
    raw = (result / "kernel.jsonl").open("w")
    for ei, entry in enumerate(timing):
        for wi, (workload, size) in enumerate(configs):
            for repeat in range(-1, args.repeats):
                ordered = VARIANTS if (repeat + wi + ei) % 2 else VARIANTS[::-1]
                for variant in ordered:
                    command = small_parent(ROOT / entry["binary"], "sample", variant, workload, str(size))
                    record, data = execute(command, result, required=("full_ns", "construct_ns", "calipers_ns", "recovery_ns", "objective_digest", "input_digest", "hull_sum", "cases", "points"))
                    row = dict(compiler=entry["compiler"], mode=entry["mode"], workload=workload, size=size,
                               variant=variant, repeat=repeat, warmup=repeat < 0, command=command, **data)
                    raw.write(json.dumps(row) + "\n"); raw.flush(); rows.append(row)
                    identity = tuple(data[k] for k in ["input_digest", "objective_digest", "cases", "points", "hull_sum"])
                    if identity != expected.setdefault((workload, size), identity):
                        write(result / "failed-profile.json", dict(row=row, execution=record))
                        raise RuntimeError("Kernel identity mismatch")
    raw.close()
    io_rows, io_expected = [], {}
    raw = (result / "io.jsonl").open("w")
    for ei, entry in enumerate(timing):
        for workload in ["random", "tiny"]:
            for repeat in range(-1, args.repeats):
                ordered = VARIANTS if (repeat + ei) % 2 else VARIANTS[::-1]
                for variant in ordered:
                    for kind in (["fast", "iostream"] if repeat % 2 else ["iostream", "fast"]):
                        command = small_parent(ROOT / entry["binary"], "io", variant, kind)
                        record, data = execute(command, result, input_path=ROOT / prepared["inputs"][workload]["path"],
                            json_stream="stderr", required=("parse_ns", "solve_ns", "format_ns", "rss_kib", "cases", "objective_digest"))
                        row = dict(compiler=entry["compiler"], mode=entry["mode"], workload=workload, variant=variant,
                                   io=kind, repeat=repeat, warmup=repeat < 0, command=command, end_to_end_ns=record["process_ns"],
                                   output_sha256=hashlib.sha256(record["stdout"].encode()).hexdigest(), **data)
                        raw.write(json.dumps(row) + "\n"); raw.flush(); io_rows.append(row)
                        identity = (data["cases"], data["objective_digest"])
                        if identity != io_expected.setdefault(workload, identity) or len(record["stdout"].splitlines()) != data["cases"]:
                            write(result / "failed-io.json", dict(row=row, execution=record))
                            raise RuntimeError("I/O objective/output mismatch")
    raw.close()
    summaries = []
    for family, source, keys in [("kernel", rows, ["compiler", "mode", "workload", "size", "variant"]),
                                  ("io", io_rows, ["compiler", "mode", "workload", "variant", "io"])]:
        groups = sorted({tuple(row[key] for key in keys) for row in source})
        for group in groups:
            chosen = [row for row in source if not row["warmup"] and tuple(row[key] for key in keys) == group]
            summaries.append(dict(family=family, **dict(zip(keys, group)), summary=stats(chosen), samples=len(chosen)))
    write(result / "summary.json", summaries)
    metadata.update(finished_at=now(), kernel_profiles=len(rows), io_profiles=len(io_rows),
                    warmup_profiles=sum(row["warmup"] for row in rows + io_rows))
    write(result / "metadata.json", metadata)
    print(f"Completed {len(rows)} kernel and {len(io_rows)} I/O profiles, including warmups.")


def diagnostics(args):
    out, prepared = checked(args)
    result = out / "diagnostics"
    if result.exists():
        raise RuntimeError("Diagnostics already exist")
    result.mkdir()
    write(result / "metadata.json", dict(started_at=now(), environment=environment(result),
          conditions="Separate allocation/RSS runs; no timing conclusions. Allocation counts/bytes cover full calls only, excluding input and answer storage, staged/metadata calls. Bytes are cumulative requested payload, not live/peak usage. RSS is whole process high water including input, all stages, runtime and allocator retention; small shell avoids Python RSS inheritance."))
    rows = []
    identities = {}
    measurement = out / "measurement/kernel.jsonl"
    if measurement.exists():
        for line in measurement.read_text().splitlines():
            row = json.loads(line)
            identities[(row["workload"], row["size"])] = tuple(row[k] for k in ["input_digest", "objective_digest", "cases", "points"])
    for entry in prepared["entries"]:
        if not entry["diagnostic"]:
            continue
        for workload in ["random", "parabola", "all-same", "late-different", "tiny", "tiny1", "tiny2"]:
            for variant in VARIANTS:
                command = small_parent(ROOT / entry["binary"], "sample", variant, workload, str(max(prepared["sizes"])))
                record, data = execute(command, result, required=("allocation_calls", "allocation_bytes", "objective_digest", "input_digest", "rss_kib"))
                rows.append(dict(compiler=entry["compiler"], mode=entry["mode"], workload=workload, variant=variant,
                                 command=command, **data))
                write(result / "allocation.json", rows)
                identity = tuple(data[k] for k in ["input_digest", "objective_digest", "cases", "points"])
                if identity != identities.setdefault((workload, max(prepared["sizes"])), identity):
                    write(result / "failed-allocation.json", dict(row=rows[-1], execution=record))
                    raise RuntimeError("Allocation profile objective mismatch")
    memory = []
    for entry in prepared["entries"]:
        if entry["diagnostic"]:
            continue
        for workload in ["random", "parabola", "all-same"]:
            for variant in VARIANTS:
                command = small_parent(ROOT / entry["binary"], "io", variant, "fast")
                record, data = execute(command, result, input_path=ROOT / prepared["inputs"][workload]["path"],
                                       json_stream="stderr", required=("rss_kib", "cases", "objective_digest"))
                memory.append(dict(compiler=entry["compiler"], mode=entry["mode"], workload=workload, variant=variant,
                                   command=command, rss_kib=data["rss_kib"], cases=data["cases"], objective_digest=data["objective_digest"]))
                write(result / "memory.json", memory)
                identity = identities[(workload, max(prepared["sizes"]))]
                if (data["objective_digest"], data["cases"]) != identity[1:3]:
                    write(result / "failed-memory.json", dict(row=memory[-1], execution=record))
                    raise RuntimeError("RSS profile objective mismatch")
    print(f"Completed {len(rows)} allocation and {len(memory)} separate RSS profiles; no timings used.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=["prepare", "run", "diagnostics"])
    parser.add_argument("--experiment", default="initial")
    parser.add_argument("--compilers", nargs="+", default=["g++", "clang++"])
    parser.add_argument("--sizes", nargs="+", type=int, default=[2048, 200000])
    parser.add_argument("--tiny-size", type=int, default=200000)
    parser.add_argument("--repeats", type=int, default=5)
    args = parser.parse_args()
    if pathlib.Path(args.experiment).name != args.experiment or min(args.sizes) < 2 or max(args.sizes) > 10000000 or args.repeats < 3:
        parser.error("invalid experiment, sizes, or repetition count")
    {"prepare": prepare, "run": run, "diagnostics": diagnostics}[args.action](args)


if __name__ == "__main__":
    main()
