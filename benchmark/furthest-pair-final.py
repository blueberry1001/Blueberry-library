#!/usr/bin/env python3
"""Paired exact-production-header confirmation after the initial layout decision."""
import argparse
import csv
import hashlib
import importlib.util
import json
import pathlib
import shutil
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]
HELPER = ROOT / "benchmark/furthest-pair-resume.py"
spec = importlib.util.spec_from_file_location("furthest_resume", HELPER)
h = importlib.util.module_from_spec(spec)
spec.loader.exec_module(h)
BASE = ROOT / "benchmark/results/furthest-pair-resume"
OUT = BASE / "final"
INITIAL = BASE / "initial"
SOURCE = ROOT / "benchmark/furthest-pair-resume.cpp"
VARIANTS = ["baseline", "final"]


def checked():
    prepared = json.loads((OUT / "prepared.json").read_text())
    for path, digest in prepared["bound_sha256"].items():
        if h.sha(ROOT / path) != digest:
            raise RuntimeError(f"Bound source/header/binary changed: {path}")
    if h.sha(OUT / "small-checks.json") != prepared["small_checks_sha256"]:
        raise RuntimeError("Small gate changed")
    return prepared


def prepare(args):
    initial = json.loads((INITIAL / "prepared.json").read_text())
    if h.sha(ROOT / h.HEADERS[0]) != args.production_sha256:
        raise RuntimeError("Live production header does not match the frozen hash")
    if h.sha(SOURCE) != initial["source_sha256"]:
        raise RuntimeError("Common initial harness changed")
    if OUT.exists():
        raise RuntimeError("Refusing to replace final evidence directory")
    OUT.mkdir()
    build = ROOT / ".build/furthest-pair-resume/final"
    if build.exists():
        raise RuntimeError("Refusing to replace final binary directory")
    build.mkdir(parents=True)
    shutil.copy2(pathlib.Path(__file__), OUT / "runner.py")
    shutil.copy2(HELPER, OUT / "shared-runner.py")
    prepared = dict(created_at=h.now(), git_head=subprocess.check_output(
        ["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
        production_header_sha256=args.production_sha256,
        baseline_header_sha256=initial["production_header_sha256"],
        initial_prepared_sha256=h.sha(INITIAL / "prepared.json"),
        sizes=initial["sizes"], tiny_size=initial["tiny_size"], entries=[], bound_sha256={})
    bound = [pathlib.Path(__file__), HELPER, SOURCE, OUT / "runner.py", OUT / "shared-runner.py"]
    bound += [ROOT / name for name in h.HEADERS]
    for variant in VARIANTS:
        snapshot = OUT / variant
        snapshot.mkdir()
        shutil.copy2(SOURCE, snapshot / SOURCE.name)
        if h.sha(snapshot / SOURCE.name) != initial["source_sha256"]:
            raise RuntimeError("Copied harness does not match the frozen initial source")
        for name in h.HEADERS:
            target = snapshot / name
            target.parent.mkdir(parents=True, exist_ok=True)
            source = INITIAL / "snapshot" / name if variant == "baseline" else ROOT / name
            if variant == "baseline" and h.sha(source) != initial["snapshot_sha256"]["snapshot/" + name]:
                raise RuntimeError("Initial baseline snapshot changed")
            shutil.copy2(source, target)
            if variant == "final" and name == h.HEADERS[0] and h.sha(target) != args.production_sha256:
                raise RuntimeError("Copied final header does not match the requested production hash")
            if name != h.HEADERS[0] and h.sha(target) != initial["snapshot_sha256"]["snapshot/" + name]:
                raise RuntimeError("Non-target production header changed")
        bound += [p for p in snapshot.rglob("*") if p.is_file()]
        for compiler in ["g++", "clang++"]:
            for mode in ["release", "assert"]:
                for diagnostic in [False, True] if compiler == "g++" else [False]:
                    name = variant + "-" + compiler + "-" + mode + ("-allocation" if diagnostic else "")
                    binary = build / name
                    flags = ["-std=gnu++20", "-O2", "-Wall", "-Wextra", "-I", str(snapshot)]
                    if mode == "release":
                        flags.append("-DNDEBUG")
                    if diagnostic:
                        flags += ["-DFP_ALLOCATION_DIAGNOSTIC", "-Wno-mismatched-new-delete"]
                    compile_record = h.command_result([compiler, *flags, str(snapshot / SOURCE.name), "-o", str(binary)])
                    entry = dict(variant=variant, compiler=compiler, mode=mode, diagnostic=diagnostic,
                                 binary=str(binary.relative_to(ROOT)), compile=compile_record,
                                 compiler_version=subprocess.check_output([compiler, "--version"], text=True))
                    prepared["entries"].append(entry)
                    h.write(OUT / "compile.json", prepared["entries"])
                    if compile_record["returncode"] != 0:
                        raise RuntimeError("Compile failure retained")
                    bound.append(binary)
                    entry["binary_sha256"] = h.sha(binary)
                    entry["correctness"] = h.command_result([str(binary), "check"])
                    if mode == "assert":
                        entry["invalid_singleton"] = h.command_result([str(binary), "invalid-singleton", "blueberry"])
                    h.write(OUT / "compile.json", prepared["entries"])
                    if entry["correctness"]["returncode"] != 0 or (mode == "assert" and entry["invalid_singleton"]["returncode"] != -6):
                        raise RuntimeError("Final-code correctness/assertion gate failed")
    checks, identities = [], {}
    for entry in prepared["entries"]:
        for workload in h.LARGE + h.TINY:
            command = h.small_parent(ROOT / entry["binary"], "sample", "blueberry", workload, "64")
            record, data = h.execute(command, OUT, required=("full_ns", "input_digest", "objective_digest", "cases", "points", "hull_sum"))
            row = dict(variant=entry["variant"], compiler=entry["compiler"], mode=entry["mode"],
                       diagnostic=entry["diagnostic"], workload=workload, command=command, **data)
            checks.append(row)
            h.write(OUT / "small-checks.json", checks)
            identity = tuple(data[k] for k in ["input_digest", "objective_digest", "cases", "points", "hull_sum"])
            if identity != identities.setdefault(workload, identity):
                h.write(OUT / "failed-small-profile.json", dict(row=row, execution=record))
                raise RuntimeError("Small paired objective mismatch")
    if h.sha(SOURCE) != initial["source_sha256"] or h.sha(ROOT / h.HEADERS[0]) != args.production_sha256:
        raise RuntimeError("Live harness or production header changed during preparation")
    for name in h.HEADERS:
        if h.sha(ROOT / name) != h.sha(OUT / "final" / name):
            raise RuntimeError("Live dependency differs from the compiled final snapshot")
    prepared["small_checks_sha256"] = h.sha(OUT / "small-checks.json")
    prepared["bound_sha256"] = {str(p.relative_to(ROOT)): h.sha(p) for p in bound}
    h.write(OUT / "prepared.json", prepared)
    print(f"Prepared {len(prepared['entries'])} final/baseline binaries and {len(checks)} small public-call profiles. No timings.")


def run(args):
    prepared = checked()
    out = OUT / "measurement"
    if out.exists():
        raise RuntimeError("Paired measurement already exists")
    out.mkdir()
    metadata = dict(started_at=h.now(), environment=h.environment(out), prepared_sha256=h.sha(OUT / "prepared.json"),
                    repeats=5, warmups=1,
                    conditions="Exactly paired direct public function calls from baseline/final header snapshots. Unchanged harness, inputs, dependencies and flags. Only full_ns is adoption evidence; separately executed construction/calipers/recovery proxies are retained raw but not interpreted as changed production stages. No repeated layout or I/O timing matrix.")
    h.write(out / "metadata.json", metadata)
    entries = {(e["compiler"], e["mode"], e["variant"]): e for e in prepared["entries"] if not e["diagnostic"]}
    configurations = [(w, n) for n in prepared["sizes"] for w in h.LARGE] + [(w, prepared["tiny_size"]) for w in h.TINY]
    rows, identities = [], {}
    with (out / "kernel.jsonl").open("w") as raw:
        for ci, compiler in enumerate(["g++", "clang++"]):
            for mi, mode in enumerate(["release", "assert"]):
                for wi, (workload, size) in enumerate(configurations):
                    for repeat in range(-1, 5):
                        ordered = VARIANTS if (repeat + wi + mi + ci) % 2 else VARIANTS[::-1]
                        for variant in ordered:
                            entry = entries[compiler, mode, variant]
                            command = h.small_parent(ROOT / entry["binary"], "sample", "blueberry", workload, str(size))
                            record, data = h.execute(command, out, required=("full_ns", "input_digest", "objective_digest", "cases", "points", "hull_sum"))
                            row = dict(compiler=compiler, mode=mode, workload=workload, size=size, variant=variant,
                                       repeat=repeat, warmup=repeat < 0, command=command, **data)
                            raw.write(json.dumps(row) + "\n"); raw.flush(); rows.append(row)
                            identity = tuple(data[k] for k in ["input_digest", "objective_digest", "cases", "points", "hull_sum"])
                            if identity != identities.setdefault((workload, size), identity):
                                h.write(out / "failed-profile.json", dict(row=row, execution=record))
                                raise RuntimeError("Paired full-call objective mismatch")
    comparisons = []
    keys = ["compiler", "mode", "workload", "size"]
    for group in sorted({tuple(row[k] for k in keys) for row in rows}):
        stats = {}
        for variant in VARIANTS:
            samples = [row for row in rows if not row["warmup"] and row["variant"] == variant and tuple(row[k] for k in keys) == group]
            stats[variant] = h.stats([dict(full_ns=row["full_ns"]) for row in samples])["full_ns"]
        before, after = stats["baseline"], stats["final"]
        comparisons.append(dict(zip(keys, group), baseline_median_ns=before["median"], baseline_min_ns=before["min"],
            baseline_max_ns=before["max"], final_median_ns=after["median"], final_min_ns=after["min"], final_max_ns=after["max"],
            delta_percent=100 * (after["median"] / before["median"] - 1),
            separated_faster=after["max"] < before["min"], separated_slower=after["min"] > before["max"]))
    h.write(out / "comparisons.json", comparisons)
    with (out / "comparisons.csv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(comparisons[0]), lineterminator="\n")
        writer.writeheader(); writer.writerows(comparisons)
    metadata.update(finished_at=h.now(), profiles=len(rows), warmup_profiles=sum(row["warmup"] for row in rows))
    h.write(out / "metadata.json", metadata)
    print(f"Completed {len(rows)} paired public-call profiles, including retained warmups.")


def diagnostics(args):
    prepared = checked()
    out = OUT / "diagnostics"
    if out.exists():
        raise RuntimeError("Paired diagnostics already exist")
    out.mkdir()
    h.write(out / "metadata.json", dict(started_at=h.now(), environment=h.environment(out),
        conditions="12 separate allocation-only profiles: baseline/final × release/assert × all-same/tiny/late-different. Instrumented timing values unused; counts exclude input/answers and later staged/metadata work."))
    identities, rows = {}, []
    for line in (OUT / "measurement/kernel.jsonl").read_text().splitlines():
        row = json.loads(line)
        if row["size"] == max(prepared["sizes"]):
            identities[row["workload"]] = tuple(row[k] for k in ["input_digest", "objective_digest", "cases", "points"])
    for entry in prepared["entries"]:
        if not entry["diagnostic"]:
            continue
        for workload in ["all-same", "tiny", "late-different"]:
            command = h.small_parent(ROOT / entry["binary"], "sample", "blueberry", workload, str(max(prepared["sizes"])))
            record, data = h.execute(command, out, required=("allocation_calls", "allocation_bytes", "input_digest", "objective_digest", "cases", "points"))
            row = dict(variant=entry["variant"], mode=entry["mode"], compiler=entry["compiler"], workload=workload, command=command, **data)
            rows.append(row); h.write(out / "allocation.json", rows)
            if tuple(data[k] for k in ["input_digest", "objective_digest", "cases", "points"]) != identities[workload]:
                h.write(out / "failed-profile.json", dict(row=row, execution=record))
                raise RuntimeError("Paired diagnostic objective mismatch")
    print(f"Completed {len(rows)} separate paired allocation profiles; timings unused.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=["prepare", "run", "diagnostics"])
    parser.add_argument("--production-sha256")
    args = parser.parse_args()
    if args.action == "prepare" and not args.production_sha256:
        parser.error("prepare requires the frozen production SHA256")
    {"prepare": prepare, "run": run, "diagnostics": diagnostics}[args.action](args)


if __name__ == "__main__":
    main()
