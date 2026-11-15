#!/usr/bin/env python3
import argparse
import hashlib
import json
from pathlib import Path
import platform
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
SHAPES = ("empty", "full", "half", "sparse", "cluster")


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--candidate", type=Path, default=ROOT / "benchmark/fast-set-assembly-sse2.hpp.txt")
    parser.add_argument("--compare", type=Path)
    parser.add_argument("--compare2", type=Path)
    parser.add_argument("--runs", type=int, default=3)
    parser.add_argument("--n", type=int, default=10000000)
    parser.add_argument("--q", type=int, default=600000)
    parser.add_argument("--native", action="store_true")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.runs < 1 or args.n < 1 or args.q < 0:
        parser.error("runs and n must be positive; q must be nonnegative")
    build = ROOT / ".build/fast-set-assembly" / Path(args.compiler).name
    build.mkdir(parents=True, exist_ok=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    baseline = ROOT / "benchmark/fast-set-assembly-baseline.hpp.txt"
    driver = ROOT / "benchmark/fast-set-assembly.cpp"
    headers = {"baseline": baseline, "candidate": args.candidate}
    if args.compare:
        headers["compare"] = args.compare
    if args.compare2:
        headers["compare2"] = args.compare2
    flags = ["-std=gnu++20", "-O2", "-DNDEBUG"] + (["-march=native"] if args.native else [])
    cpu = next((line.split(":", 1)[1].strip() for line in Path("/proc/cpuinfo").read_text().splitlines()
                if line.startswith("model name")), platform.processor())
    result = {"timestamp_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
              "cpu": cpu, "platform": platform.platform(), "flags": flags, "n": args.n, "q": args.q,
              "runs": args.runs, "seed": 1729, "sha256": {k: sha(v) for k, v in headers.items()},
              "driver_sha256": sha(driver), "measurements": [], "status": "not_completed"}

    def save():
        args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")

    try:
        result["compiler"] = subprocess.check_output([args.compiler, "--version"], text=True).splitlines()[0]
        for name, path in headers.items():
            include = build / name
            include.mkdir(exist_ok=True)
            (include / "fast-set-assembly.hpp").write_bytes(path.read_bytes())
            command = [args.compiler, *flags, "-I", str(include), str(driver), "-o", str(build / (name + ".out"))]
            print("compile", " ".join(command), flush=True)
            subprocess.run(command, check=True)
        for shape in SHAPES:
            expected = None
            for run in range(-1, args.runs):
                names = list(headers)
                offset = (run + 1) % len(names)
                names = names[offset:] + names[:offset]
                for name in names:
                    cmd = ["/usr/bin/time", "-f", "%M", str(build / (name + ".out")), str(args.n), str(args.q), shape]
                    proc = subprocess.run(cmd, text=True, capture_output=True, check=True)
                    sample = json.loads(proc.stdout)
                    if expected is None:
                        expected = sample["checksum"]
                    if sample["checksum"] != expected:
                        raise ValueError(f"checksum mismatch for {shape}/{name}/{run}")
                    sample.update(variant=name, run=run + 1,
                                  peak_rss_kib=int(proc.stderr.strip().splitlines()[-1]))
                    print(json.dumps(sample), flush=True)
                    if run >= 0:
                        result["measurements"].append(sample)
                        save()
        result["summary"] = [{"shape": shape, "variant": name,
                              "build_median_ms": statistics.median(s["build_ms"] for s in result["measurements"]
                                                                     if s["shape"] == shape and s["variant"] == name),
                              "ops_median_ms": statistics.median(s["operations_ms"] for s in result["measurements"]
                                                                   if s["shape"] == shape and s["variant"] == name),
                              "rss_median_kib": statistics.median(s["peak_rss_kib"] for s in result["measurements"]
                                                                   if s["shape"] == shape and s["variant"] == name)}
                             for shape in SHAPES for name in headers]
        result["status"] = "completed"
    except Exception as error:
        result["error"] = str(error)
        raise
    finally:
        save()


if __name__ == "__main__":
    main()
