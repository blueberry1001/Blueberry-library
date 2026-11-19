import argparse
import datetime
import hashlib
import json
import os
import platform
import random
import re
import statistics
import subprocess
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent


def sha(path):
    h = hashlib.sha256()
    with path.open("rb") as f:
        for block in iter(lambda: f.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--cpu", type=int, required=True)
    p.add_argument("--compiler", default="g++")
    p.add_argument("--problem", type=Path, required=True)
    p.add_argument("--runs", type=int, default=5)
    p.add_argument("--warmups", type=int, default=1)
    p.add_argument("--output", required=True)
    args = p.parse_args()
    allowed = sorted(os.sched_getaffinity(0))
    if args.cpu not in allowed:
        raise ValueError("CPU outside allowed affinity")
    os.sched_setaffinity(0, {args.cpu})
    problem = args.problem.resolve()
    out = Path(args.output).resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    with out.open("x", encoding="utf-8") as f:
        json.dump({"status": "started"}, f)
    build = out.parent / (out.stem + "-bin")
    info = Path("/proc/cpuinfo").read_text(encoding="utf-8")
    model = re.search(r"^model name\s*:\s*(.+)$", info, re.MULTILINE)
    data = {"status": "started", "args": {**vars(args), "problem": str(problem)},
            "affinity": sorted(os.sched_getaffinity(0)), "available_affinity": allowed,
            "cpu_model": model.group(1) if model else None, "platform": platform.platform(),
            "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
            "commands": [], "samples": []}

    def save():
        out.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")

    def command(argv):
        result = subprocess.run([str(x) for x in argv], capture_output=True, text=True)
        item = {"argv": [str(x) for x in argv], "returncode": result.returncode,
                "stdout": result.stdout, "stderr": result.stderr}
        data["commands"].append(item)
        save()
        if result.returncode:
            raise RuntimeError(f"command failed: {argv}")
        return item

    try:
        if args.runs < 1 or args.warmups < 0:
            raise ValueError("invalid run count")
        build.mkdir(exist_ok=False)
        checker = problem / "checker"
        inputs = sorted((problem / "in").glob("*.in"))
        if not checker.is_file() or not inputs:
            raise FileNotFoundError("official checker and generated input directory required")
        cases = [(path, problem / "out" / (path.stem + ".out")) for path in inputs]
        if not all(answer.is_file() for _, answer in cases):
            raise FileNotFoundError("missing official answer")
        sources = [HERE / "official_verify.cpp", Path(__file__)]
        for variant in ("baseline", "candidate"):
            sources += list((HERE / variant / "blueberry/data-structure").glob("*.hpp"))
        data["source_sha256"] = {str(path.relative_to(HERE)): sha(path) for path in sources}
        data["problem_hashes"] = {"checker": sha(checker), "checker.cpp": sha(problem / "checker.cpp"),
                                  "info.toml": sha(problem / "info.toml"),
                                  "cases": {x.name: {"input": sha(x), "answer": sha(y)} for x, y in cases}}
        command([args.compiler, "--version"])
        command(["git", "-C", problem, "rev-parse", "HEAD"])
        binaries = {}
        for variant in ("baseline", "candidate"):
            binaries[variant] = build / variant
            command([args.compiler, "-std=gnu++20", "-O2", "-DNDEBUG", "-I", HERE / variant,
                     HERE / "official_verify.cpp", "-o", binaries[variant]])
        data["binary_sha256"] = {v: sha(b) for v, b in binaries.items()}
        rng = random.Random(20261118)
        for case, answer in cases:
            for iteration in range(-args.warmups, args.runs):
                variants = ["baseline", "candidate"]
                rng.shuffle(variants)
                for variant in variants:
                    result_path = build / f"{variant}.out"
                    timing_path = build / "time.txt"
                    argv = ["/usr/bin/time", "-f", "%e %M", "-o", str(timing_path), str(binaries[variant])]
                    started = datetime.datetime.now(datetime.timezone.utc).isoformat()
                    t0 = time.perf_counter()
                    with case.open("rb") as input_file, result_path.open("wb") as output_file:
                        result = subprocess.run(argv, stdin=input_file, stdout=output_file, stderr=subprocess.PIPE)
                    wall = time.perf_counter() - t0
                    timing = timing_path.read_text(encoding="utf-8").strip()
                    check = subprocess.run([str(checker), str(case), str(result_path), str(answer)], capture_output=True, text=True)
                    sample = {"case": case.name, "variant": variant, "iteration": iteration,
                              "warmup": iteration < 0, "started_utc": started, "wall_s": wall,
                              "returncode": result.returncode, "stderr": result.stderr.decode("utf-8", "replace"),
                              "time_raw": timing, "output_sha256": sha(result_path),
                              "checker_returncode": check.returncode, "checker_stdout": check.stdout, "checker_stderr": check.stderr}
                    data["samples"].append(sample)
                    save()
                    if result.returncode or check.returncode or not re.fullmatch(r"[0-9.]+ [0-9]+", timing):
                        raise RuntimeError(f"execution/checker failed: {sample}")
        data["summary"] = {}
        for case, _ in cases:
            data["summary"][case.name] = {}
            for variant in ("baseline", "candidate"):
                samples = [x for x in data["samples"] if x["case"] == case.name and x["variant"] == variant and not x["warmup"]]
                values = [x["wall_s"] for x in samples]
                memory = [int(x["time_raw"].split()[1]) for x in samples]
                data["summary"][case.name][variant] = {"wall_median_s": statistics.median(values), "wall_min_s": min(values),
                                                       "rss_median_kib": statistics.median(memory)}
        data["status"] = "complete"
    except Exception as e:
        data["status"] = "failed"
        data["error"] = repr(e)
        raise
    finally:
        data["finished_utc"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
        save()


if __name__ == "__main__":
    main()
