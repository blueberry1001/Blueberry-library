#!/usr/bin/env python3
"""Prepare immutable baseline/candidate headers, then measure only in a quiet window."""

import argparse
import difflib
import hashlib
import json
import pathlib
import platform
import statistics
import subprocess
from datetime import datetime, timezone


ROOT = pathlib.Path(__file__).resolve().parents[1]
BUILD = ROOT / ".build/static-top-tree-sam"
OUT = ROOT / "benchmark/results/static-top-tree-sam"
SOURCE = ROOT / "benchmark/static-top-tree-sam.cpp"
HEADERS = ["blueberry/graph/static-top-tree.hpp", "blueberry/string/suffix-automaton.hpp"]
BASELINE = "9c2914f"


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write(name, data):
    (OUT / name).write_text(json.dumps(data, indent=2) + "\n")


def replace_once(text, before, after):
    if text.count(before) != 1:
        raise RuntimeError(f"Candidate edit must match once: {before!r}")
    return text.replace(before, after, 1)


def candidate_source(path, source):
    if path.endswith("static-top-tree.hpp"):
        source = replace_once(source, "#include <functional>\n", "#include <cstddef>\n")
        source = replace_once(source, "if(items.empty())return -1;\n", "if(items.empty())return -1;\n    if(items.size()==1)return items[0];\n")
        source = replace_once(source, "if(l==r)return -1;\n", "if(l==r)return -1;\n      if(l+1==r)return items[l];\n")
    else:
        source = replace_once(source, "#include <type_traits>\n", "#include <type_traits>\n#include <utility>\n")
        source = replace_once(source, "states_.push_back(copied);", "states_.push_back(std::move(copied));")
    return source


def prepare(args):
    BUILD.mkdir(parents=True, exist_ok=True)
    OUT.mkdir(parents=True, exist_ok=True)
    if (OUT / "prepared.json").exists():
        raise RuntimeError("Preparation exists; preserve results before replacing an immutable experiment")
    base = subprocess.check_output(["git", "rev-parse", BASELINE], cwd=ROOT, text=True).strip()
    headers = {}
    patch = []
    for path in HEADERS:
        before = subprocess.check_output(["git", "show", f"{base}:{path}"], cwd=ROOT, text=True)
        after = candidate_source(path, before)
        patch.extend(difflib.unified_diff(before.splitlines(keepends=True), after.splitlines(keepends=True),
                                         fromfile="a/" + path, tofile="b/" + path))
        for variant, contents in [("baseline", before), ("candidate", after)]:
            destination = BUILD / variant / "include" / path
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_text(contents)
            headers[str(destination.relative_to(ROOT))] = digest(destination)
            (OUT / f"{variant}-{pathlib.Path(path).name}.txt").write_text(contents)
    (OUT / "candidate.patch").write_text("".join(patch))
    entries = []
    for compiler in args.compilers:
        modes = ["timing", "allocations"] if compiler == args.compilers[0] else ["timing"]
        for mode in modes:
            for variant in ["baseline", "candidate"]:
                binary = BUILD / (pathlib.Path(compiler).name + "-" + mode + "-" + variant)
                command = [compiler, "-std=gnu++20", "-O2", "-Wall", "-Wextra",
                           "-I", str(BUILD / variant / "include"), "-I", str(ROOT)]
                if not args.assertions:
                    command.append("-DNDEBUG")
                if mode == "allocations":
                    command.append("-DPROFILE_ALLOCATIONS")
                command.extend([str(SOURCE), "-o", str(binary)])
                result = subprocess.run(command, capture_output=True, text=True)
                entry = dict(compiler=compiler, variant=variant, mode=mode, command=command,
                             version=subprocess.check_output([compiler, "--version"], text=True),
                             binary=str(binary.relative_to(ROOT)), returncode=result.returncode,
                             stdout=result.stdout, stderr=result.stderr,
                             binary_sha256=digest(binary) if result.returncode == 0 else None)
                entries.append(entry)
                write("compile.json", entries)
                result.check_returncode()
    write("prepared.json", dict(created_at=datetime.now(timezone.utc).isoformat(), baseline_commit=base,
                                assertions_enabled=args.assertions,
                                source_sha256=digest(SOURCE), header_sha256=headers, entries=entries))
    print("Prepared four timing binaries and two allocation-diagnostic binaries; no runs started.")


def checked_prepared():
    prepared = json.loads((OUT / "prepared.json").read_text())
    if digest(SOURCE) != prepared["source_sha256"]:
        raise RuntimeError("Harness changed since preparation")
    for relative, expected in prepared["header_sha256"].items():
        if digest(ROOT / relative) != expected:
            raise RuntimeError("Immutable header changed: " + relative)
    for entry in prepared["entries"]:
        if digest(ROOT / entry["binary"]) != entry["binary_sha256"]:
            raise RuntimeError("Binary changed since preparation")
    return prepared


def cases(small=False):
    result = []
    for shape in ["path", "star", "balanced", "random", "broom"]:
        for scalar in ["u64", "mod998"]:
            result.append(["tree", shape, scalar, 127 if small else 100000, 211 if small else 200000])
    for shape in ["random4", "random26", "periodic", "equal", "int"]:
        result.append(["sam", shape, "generic", 127 if small else 200000, 211 if small else 20000])
    return result


def execute(entry, case):
    # Spawn beneath a small shell so ru_maxrss is not floored by Python's RSS.
    command = ["/bin/sh", "-c", '"$@"; result=$?; exit "$result"', "performance-profile",
               str(ROOT / entry["binary"]), *map(str, case)]
    completed = subprocess.run(command, capture_output=True, text=True, timeout=120, check=True)
    row = json.loads(completed.stdout)
    row.update(compiler=entry["compiler"], variant=entry["variant"], mode=entry["mode"], scalar=case[2])
    return row


def check(args):
    prepared = checked_prepared()
    expected = {}
    rows = []
    for entry in prepared["entries"]:
        for case in cases(small=True):
            row = execute(entry, case)
            # Durations from correctness preparation are intentionally discarded.
            row = {key: value for key, value in row.items() if not key.endswith("_ns")}
            identity = (row["input_hash"], row["checksum"])
            key = tuple(case)
            if identity != expected.setdefault(key, identity):
                raise RuntimeError("Small-case checksum mismatch")
            rows.append(row)
    write("correctness.json", rows)
    print(f"All {len(rows)} small-case profiles agreed; no timings retained.")


def quiet_snapshot():
    snapshot = subprocess.check_output(["ps", "-eo", "pid,pcpu,stat,comm"], text=True)
    (OUT / "quiet-processes.txt").write_text(snapshot)
    for line in snapshot.splitlines():
        fields = line.split()
        if len(fields) >= 4 and fields[3] in {"cc1plus", "clang", "clang++", "clang-19", "g++", "g++-14"}:
            if "T" not in fields[2] and "Z" not in fields[2]:
                raise RuntimeError("Active compiler; coordinate another quiet window")


def run(args):
    prepared = checked_prepared()
    if (OUT / "raw.json").exists():
        raise RuntimeError("Timing output exists; preserve it before another experiment")
    quiet_snapshot()
    entries = [entry for entry in prepared["entries"] if entry["mode"] == "timing"]
    expected = {}
    rows = []
    for case in cases():
        for repeat in range(-1, args.repeats):
            # Alternate before/after and compiler order at every repetition.
            for entry in entries if repeat % 2 else entries[::-1]:
                row = execute(entry, case)
                identity = (row["input_hash"], row["checksum"])
                if identity != expected.setdefault(tuple(case), identity):
                    raise RuntimeError("Profile checksum mismatch")
                row.update(repeat=repeat, warmup=repeat < 0)
                # These fields only have meaning in the separate instrumented builds.
                row = {key: value for key, value in row.items()
                       if not key.endswith("_allocations") and not key.endswith("_callbacks")}
                rows.append(row)
                write("raw.json", rows)
    summary = []
    for case in cases():
        for entry in entries:
            selected = [row for row in rows if not row["warmup"] and
                        (row["family"], row["shape"], row["scalar"], row["compiler"], row["variant"]) ==
                        (*case[:3], entry["compiler"], entry["variant"])]
            measured = [field for field in selected[0] if field.endswith("_ns")]
            metrics = {field: dict(median=statistics.median(row[field] for row in selected),
                                   min=min(row[field] for row in selected),
                                   max=max(row[field] for row in selected)) for field in measured}
            summary.append(dict(family=case[0], shape=case[1], scalar=case[2], compiler=entry["compiler"],
                                variant=entry["variant"], metrics=metrics,
                                peak_rss_kib=max(row["rss_kib"] for row in selected)))
    write("results.json", dict(date=datetime.now(timezone.utc).isoformat(), prepared=prepared,
                               platform=platform.platform(), cpu=subprocess.check_output(["lscpu"], text=True),
                               repeats=args.repeats, warmups=1, summary=summary,
                               conditions=f"Coordinated quiet window, {args.repeats} alternating before/after samples after warmup. "
                                          "Assertion state is recorded in each immutable compile command; experiments kept separate. "
                                          "Input generation, process startup and output excluded. "
                                          "Tree operation phase includes one all_prod checksum after every set. "
                                          "SAM build, first lazy count, repeated count/contains and LCS are separate. "
                                          "Destruction excluded. Process RSS includes generated inputs. Shared-host activity possible."))
    print(f"Completed {len(rows)} profiles including warmups; all checksums agree.")


def allocations(args):
    prepared = checked_prepared()
    entries = [entry for entry in prepared["entries"] if entry["mode"] == "allocations"]
    rows = []
    expected = {}
    for case in cases():
        for entry in entries:
            row = execute(entry, case)
            row = {key: value for key, value in row.items() if not key.endswith("_ns")}
            identity = (row["input_hash"], row["checksum"])
            if identity != expected.setdefault(tuple(case), identity):
                raise RuntimeError("Diagnostic checksum mismatch")
            if case[0] == "tree":
                calls = (row["build_callbacks"], row["operation_callbacks"])
                if calls != expected.setdefault(("callbacks", *case), calls):
                    raise RuntimeError("StaticTopTree callback count changed")
            rows.append(row)
            write("allocations.json", dict(conditions="Separate instrumented runs; all duration fields discarded. "
                                         "Counts ordinary operator new/new[] calls and requested bytes; excludes input generation. "
                                         "Not peak-live allocation bytes. Callback order labels: vertex,edge,compress,rake,identity.", rows=rows))
    print(f"Completed {len(rows)} allocation profiles; tree callback counts and all checksums agree.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=["prepare", "check", "run", "allocations"])
    parser.add_argument("--compilers", nargs="+", default=["g++", "clang++"])
    parser.add_argument("--repeats", type=int, default=5)
    parser.add_argument("--assertions", action="store_true", help="prepare without -DNDEBUG")
    args = parser.parse_args()
    if args.repeats < 3:
        parser.error("at least three repetitions required")
    {"prepare": prepare, "check": check, "run": run, "allocations": allocations}[args.action](args)


if __name__ == "__main__":
    main()
