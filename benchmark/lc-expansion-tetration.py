#!/usr/bin/env python3
"""Reproduce the tetration study; downloaded upstream sources stay in .build."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import random
import statistics
import subprocess
import time
import re

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / ".build/lc-expansion-tetration"
RESULT = ROOT / "benchmark/lc-expansion-tetration-results.json"
FLAGS = ["-std=gnu++20", "-O2", "-DNDEBUG"]


def prepare():
    BUILD.mkdir(parents=True, exist_ok=True)
    compile_logs = []
    for compiler in ["g++", "clang++"]:
        for standard in ["gnu++20", "gnu++23"]:
            binary = BUILD / f"random-{compiler}-{standard}"
            command = [compiler, f"-std={standard}", "-O2", "-Wall", "-Wextra", "-Wshadow", "-Werror",
                       "-I", ".", "tests/random/tetration_mod.cpp", "-o", str(binary)]
            subprocess.run(command, cwd=ROOT, check=True)
            for seed in [12345, 1961338640]:
                subprocess.run([str(binary), str(seed)], cwd=ROOT, check=True)
        for release in [True, False]:
            name = "release" if release else "assert"
            command = [compiler, "-std=gnu++20", "-O2", *(["-DNDEBUG"] if release else []),
                       "-Wall", "-Wextra", "-Wshadow", "-Werror", "-I", ".",
                       "benchmark/lc-expansion-tetration.cpp", "-o", str(BUILD / f"benchmark-{compiler}-{name}")]
            subprocess.run(command, cwd=ROOT, check=True)
        if compiler == "g++":
            for sid in [150339, 108768, 405835]:
                source = ROOT / f".build/lc-expansion-tetration-fastest/submission-{sid}.cpp"
                command = [compiler, *FLAGS, str(source), "-o", str(BUILD / f"fastest-{sid}")]
                result = subprocess.run(command, cwd=ROOT, text=True, capture_output=True)
                compile_logs.append(dict(command=command, returncode=result.returncode, stderr=result.stderr))
                if sid == 150339 and result.returncode == 0:
                    # Normalize effective optimization/ISA flags for a comparable run.
                    normalized = BUILD / "fastest-150339-normalized.cpp"
                    normalized.write_text(re.sub(r"^#pragma GCC (?:optimize|target).*\n", "", source.read_text(), flags=re.M))
                    normalized_command = [compiler, *FLAGS, str(normalized), "-o", str(BUILD / "fastest-150339-normalized")]
                    normalized_result = subprocess.run(normalized_command, cwd=ROOT, text=True, capture_output=True)
                    compile_logs.append(dict(command=normalized_command, returncode=normalized_result.returncode,
                                             stderr=normalized_result.stderr))
    (BUILD / "compile.json").write_text(json.dumps(compile_logs, indent=2) + "\n")
    rng = random.Random(1961338640)
    for case in ["mixed", "prime", "power-two", "small-height"]:
        queries = []
        for _ in range(5000):
            a, h, m = rng.randrange(10**9 + 1), rng.randrange(10**9 + 1), rng.randrange(1, 10**9 + 1)
            if case == "prime": m = 998244353
            if case == "power-two": a, m = 2 * rng.randrange(1, 500000000), 1 << rng.randrange(1, 30)
            if case == "small-height": h = rng.randrange(5)
            queries.append(f"{a} {h} {m}")
        (BUILD / f"{case}.in").write_text("5000\n" + "\n".join(queries) + "\n")


def measure(cases, selected_modes):
    report = dict(cpu=next(line.split(":", 1)[1].strip() for line in Path("/proc/cpuinfo").read_text().splitlines()
                          if line.startswith("model name")), os=platform.platform(), flags=FLAGS,
                  seed=1961338640, count=5000, runs=3, compilers={}, results=[],
                  upstream_compile=json.loads((BUILD / "compile.json").read_text()))
    report["sources"] = {str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest()
                         for path in [ROOT / "blueberry/math/tetration-mod.hpp", Path(__file__),
                                      ROOT / "benchmark/lc-expansion-tetration.cpp"]}
    report["upstream_sources"] = {path.name: hashlib.sha256(path.read_bytes()).hexdigest()
                                  for path in (ROOT / ".build/lc-expansion-tetration-fastest").glob("submission-*.cpp")}
    modes = {0: "Euler/trial", 1: "Euler/factorize", 2: "gcd/factorize-initial", 3: "gcd/trial", 4: "gcd/factorize"}
    for compiler in ["g++", "clang++"]:
        report["compilers"][compiler] = subprocess.check_output([compiler, "--version"], text=True).splitlines()[0]
        for release in [True, False]:
            build_kind = "release" if release else "assert"
            for case in cases:
                data = (BUILD / f"{case}.in").read_bytes()
                expected = None
                jobs = [(modes[mode], [str(BUILD / f"benchmark-{compiler}-{build_kind}"), str(mode)]) for mode in selected_modes]
                if compiler == "g++" and release:
                    jobs += [(f"Fastest/{sid}", [str(BUILD / f"fastest-{sid}")]) for sid in [150339, 108768, 405835]
                             if (BUILD / f"fastest-{sid}").exists()]
                    if (BUILD / "fastest-150339-normalized").exists():
                        jobs.append(("Fastest/150339-normalized", [str(BUILD / "fastest-150339-normalized")]))
                for name, command in jobs:
                    row = dict(compiler=compiler, build=build_kind, case=case, implementation=name,
                               input_sha256=hashlib.sha256(data).hexdigest(), samples=[])
                    for run in range(3):
                        rss = BUILD / "rss.txt"
                        started = time.perf_counter()
                        result = subprocess.run(["/usr/bin/time", "-f", "%M", "-o", str(rss), *command],
                                                input=data, capture_output=True, cwd=ROOT)
                        elapsed = time.perf_counter() - started
                        if result.returncode:
                            row["failure"] = dict(run=run, returncode=result.returncode,
                                                  stderr=result.stderr.decode(errors="replace"))
                            report["results"].append(row)
                            RESULT.write_text(json.dumps(report, indent=2) + "\n")
                            raise RuntimeError(f"benchmark failed: {compiler}/{case}/{name}")
                        if expected is None: expected = result.stdout
                        if result.stdout != expected:
                            row["failure"] = dict(run=run, reason="answer mismatch")
                            report["results"].append(row)
                            RESULT.write_text(json.dumps(report, indent=2) + "\n")
                            raise RuntimeError(f"answer mismatch: {compiler}/{case}/{name}")
                        sample = dict(e2e=elapsed, max_rss_kib=int(rss.read_text()))
                        if name in modes.values(): sample.update(json.loads(result.stderr))
                        row["samples"].append(sample)
                    for key in ["e2e", "core", "parse", "format", "max_rss_kib"]:
                        values = [sample[key] for sample in row["samples"] if key in sample]
                        if values: row[key + "_median"], row[key + "_min"] = statistics.median(values), min(values)
                    report["results"].append(row)
                    RESULT.write_text(json.dumps(report, indent=2) + "\n")
                    print(compiler, build_kind, case, name, row.get("core_median"), row["e2e_median"], flush=True)
    RESULT.write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("action", choices=["prepare", "measure"])
    parser.add_argument("--cases", nargs="+", default=["mixed", "prime", "power-two", "small-height"])
    parser.add_argument("--modes", nargs="+", type=int, default=[0, 1, 2, 3, 4])
    args = parser.parse_args()
    os.chdir(ROOT)
    prepare() if args.action == "prepare" else measure(args.cases, args.modes)
