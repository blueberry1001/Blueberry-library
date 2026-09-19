#!/usr/bin/env python3
"""Reproduce the Static RMQ cost study; keeps every sample and checks full output.

Requires Linux, g++ and clang++; official inputs are read from --problem-dir.
Third-party references are optional local files, never downloaded or committed here.
"""
import argparse
import hashlib
import itertools
import json
import os
from pathlib import Path
import platform
import random
import shutil
import statistics
import subprocess
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
ALGORITHMS = ["sparse", "disjoint", "sqrt", "linear", "block16", "mask32"]
CASES = ["max_random_00", "small_width_query_00", "small_values_00"]
PROFILES = {
    "gcc-verify": ["g++", "-std=gnu++20", "-O2", "-pipe", "-Wall", "-Wextra", "-Wshadow", "-D_GLIBCXX_ASSERTIONS"],
    "gcc-release": ["g++", "-std=gnu++20", "-O2", "-DNDEBUG"],
    "clang-release": ["clang++", "-std=gnu++20", "-O2", "-DNDEBUG"],
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--problem-dir", type=Path, required=True)
    p.add_argument("--reference-dir", type=Path)
    p.add_argument("--output", type=Path, required=True)
    p.add_argument("--repeats", type=int, default=5)
    args = p.parse_args()
    if args.repeats < 3:
        p.error("at least three measured repetitions required")
    args.output.mkdir(parents=True, exist_ok=False)
    cpu = min(os.sched_getaffinity(0))
    os.sched_setaffinity(0, {cpu})
    source = ROOT / "benchmarks/static-rmq-cost-breakdown.cpp"
    tracked = [source, Path(__file__).resolve(), *[ROOT / ("blueberry/data-structure/" + n + ".hpp")
               for n in ["sparse-table", "disjoint-sparse-table", "sqrt-tree", "linear-rmq"]]]
    references = sorted(args.reference_dir.glob("*.cpp")) if args.reference_dir else []
    before = {str(f.relative_to(ROOT)): sha(f) for f in tracked}
    env = {
        "revision": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
        "note": "Harness/prototypes are working-tree sources identified by hashes; library headers are unchanged.",
        "utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "platform": platform.platform(), "cpuinfo": Path("/proc/cpuinfo").read_text().split("\n\n")[0],
        "cpu_affinity": cpu, "profiles": PROFILES, "sources": before,
        "compiler_versions": {c: subprocess.check_output([c, "--version"], text=True) for c in ["g++", "clang++"]},
        "problem_revision": subprocess.check_output(["git", "-C", str(args.problem_dir), "rev-parse", "HEAD"], text=True).strip(),
        "cases": {name: {"input_sha256": sha(args.problem_dir / "in" / (name + ".in")),
                         "output_sha256": sha(args.problem_dir / "out" / (name + ".out"))} for name in CASES},
        "references": {f.stem: {"sha256": sha(f), "url": "https://judge.yosupo.jp/submission/" + f.stem,
                       "extra_flags": ["-mavx2"] if f.stem == "402567" else [],
                       "flags_note": "unmodified source pragmas remain active; not normalized to harness ISA/optimization"} for f in references},
        "repeats": args.repeats, "warmups_per_configuration": 1,
        "ordering": "fixed seed 917 shuffles configurations per round; sequential pinned processes",
        "output_check": "full stdout whitespace tokens equal official expected output, every run",
        "timing": "process wall includes startup/destruction/IO; phases use steady_clock; batch query excludes IO, stream query includes query input and output",
    }
    (args.output / "environment.json").write_text(json.dumps(env, indent=2) + "\n")
    samples = []
    with tempfile.TemporaryDirectory(prefix="blueberry-rmq-study-") as td:
        temp = Path(td)
        binaries = {}
        with (args.output / "compile.log").open("w") as log:
            for profile, flags in PROFILES.items():
                binary = temp / profile
                cmd = [*flags, "-I", str(ROOT), str(source), "-o", str(binary)]
                log.write(json.dumps(cmd) + "\n"); log.flush()
                subprocess.run(cmd, stdout=log, stderr=log, check=True)
                binaries[profile] = binary
            for ref in references:
                binary = temp / ("reference-" + ref.stem)
                cmd = ["g++", "-std=gnu++23", "-O2", str(ref.resolve()), "-o", str(binary)]
                if ref.stem == "402567": cmd += ["-mavx2"]
                log.write(json.dumps(cmd) + "\n"); log.flush()
                subprocess.run(cmd, stdout=log, stderr=log, check=True)
                binaries["reference-" + ref.stem] = binary
        # Linux regular files satisfy mmap references; no shell redirection tricks.
        expected = {}
        for name in CASES:
            shutil.copyfile(args.problem_dir / "in" / (name + ".in"), temp / (name + ".in"))
            expected[name] = (args.problem_dir / "out" / (name + ".out")).read_bytes().split()
        configs = list(itertools.product(["gcc-verify", "gcc-release"], ALGORITHMS,
                                        ["iostream", "buffered"], ["batch", "stream"], CASES))
        configs += list(itertools.product(["clang-release"], ALGORITHMS, ["buffered"], ["batch"], CASES))
        configs += [("reference-" + ref.stem, "original", "original", "original", name)
                    for ref in references for name in CASES]
        rng = random.Random(917)
        output = temp / "output.txt"
        def execute(config, input_path, expected_tokens):
            profile, algorithm, io, mode, name = config
            cmd = [str(binaries[profile])]
            if algorithm != "original": cmd += [algorithm, io, mode]
            rss = temp / "rss.txt"
            cmd = ["/usr/bin/time", "-f", "%M", "-o", str(rss), *cmd]
            with input_path.open("rb") as inp, output.open("wb") as out:
                start = time.perf_counter()
                proc = subprocess.run(cmd, stdin=inp, stdout=out, stderr=subprocess.PIPE, timeout=30)
                elapsed = (time.perf_counter() - start) * 1000
            if proc.returncode or output.read_bytes().split() != expected_tokens:
                raise RuntimeError(f"failed {config}: exit={proc.returncode}, stderr={proc.stderr[:2000]!r}")
            row = dict(profile=profile, algorithm=algorithm, io=io, mode=mode, case=name,
                       process_ms=elapsed, peak_rss_kib=int(rss.read_text()))
            if algorithm != "original": row.update(json.loads(proc.stderr))
            return row
        # Validate every prototype against every official case, beyond timed inputs.
        checked = 0
        for inp in sorted((args.problem_dir / "in").glob("*.in")):
            tokens = (args.problem_dir / "out" / (inp.stem + ".out")).read_bytes().split()
            for algorithm in ALGORITHMS:
                execute(("gcc-verify", algorithm, "buffered", "batch", inp.stem), inp, tokens)
                checked += 1
        (args.output / "correctness.json").write_text(json.dumps({"official_case_algorithm_pairs": checked, "passed": True}, indent=2) + "\n")
        with (args.output / "samples.jsonl").open("w") as log:
            for repeat in range(-1, args.repeats):
                order = configs.copy(); rng.shuffle(order)
                for config in order:
                    row = execute(config, temp / (config[-1] + ".in"), expected[config[-1]])
                    row["repeat"] = repeat; row["warmup"] = repeat == -1
                    log.write(json.dumps(row) + "\n"); log.flush()
                    if repeat >= 0: samples.append(row)
                print(f"round {repeat}: {len(order)} output-checked configurations", flush=True)
    if before != {str(f.relative_to(ROOT)): sha(f) for f in tracked}:
        raise RuntimeError("source changed during measurement")
    summaries = []
    for config in configs:
        keys = ["profile", "algorithm", "io", "mode", "case"]
        rows = [row for row in samples if tuple(row[k] for k in keys) == config]
        result = dict(zip(keys, config))
        for field in ["process_ms", "peak_rss_kib", "parse_ms", "build_ms", "query_ms", "write_ms", "inside_ms"]:
            vals = [row[field] for row in rows if field in row]
            if vals: result[field] = {"median": statistics.median(vals), "min": min(vals), "max": max(vals)}
        result["runs"] = len(rows); summaries.append(result)
    (args.output / "summary.json").write_text(json.dumps(summaries, indent=2) + "\n")
    print(f"complete: {len(samples)} measured samples; {len(configs)} warmups", flush=True)


if __name__ == "__main__":
    main()
