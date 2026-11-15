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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--runs", type=int, default=3)
    parser.add_argument("--repetitions", type=int, default=150000)
    parser.add_argument("--native", action="store_true")
    parser.add_argument("--public", action="store_true")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.runs < 1 or args.repetitions < 1:
        parser.error("runs and repetitions must be positive")
    build = ROOT / ".build/fast-set-assembly-small" / Path(args.compiler).name
    build.mkdir(parents=True, exist_ok=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    headers = {"baseline": ROOT / "benchmark/fast-set-assembly-baseline.hpp.txt",
               "sse2": ROOT / "benchmark/fast-set-assembly-sse2.hpp.txt",
               "runtime_avx2": ROOT / "benchmark/fast-set-assembly-avx2.hpp.txt"}
    if args.native:
        headers["compile_avx2"] = ROOT / "benchmark/fast-set-assembly-avx2-compile.hpp.txt"
    if args.public:
        headers["public"] = ROOT / "blueberry/data-structure/fast-set.hpp"
    driver = ROOT / "benchmark/fast-set-assembly-small.cpp"
    sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
    flags = ["-std=gnu++20", "-O2", "-DNDEBUG"] + (["-march=native"] if args.native else [])
    result = {"timestamp_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
              "platform": platform.platform(), "flags": flags, "runs": args.runs,
              "repetitions": args.repetitions, "sha256": {k: sha(v) for k, v in headers.items()},
              "driver_sha256": sha(driver), "measurements": [], "status": "not_completed"}

    def save():
        args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")

    try:
        result["compiler"] = subprocess.check_output([args.compiler, "--version"], text=True).splitlines()[0]
        result["cpu"] = next((s.split(":", 1)[1].strip() for s in Path("/proc/cpuinfo").read_text().splitlines()
                              if s.startswith("model name")), platform.processor())
        for name, path in headers.items():
            include = build / name
            include.mkdir(exist_ok=True)
            (include / "fast-set-assembly.hpp").write_bytes(path.read_bytes())
            command = [args.compiler, *flags, "-I", str(include), str(driver), "-o", str(build / (name + ".out"))]
            print("compile", " ".join(command), flush=True)
            subprocess.run(command, check=True)
        for n in (1, 63, 64, 65, 256, 1024):
            expected = None
            for run in range(-1, args.runs):
                names = list(headers)
                off = (run + 1) % len(names)
                for name in names[off:] + names[:off]:
                    output = subprocess.check_output([str(build / (name + ".out")), str(n), str(args.repetitions)], text=True)
                    sample = json.loads(output)
                    if expected is None:
                        expected = sample["checksum"]
                    if sample["checksum"] != expected:
                        raise ValueError(f"checksum mismatch for n={n}/{name}/{run}")
                    sample.update(variant=name, run=run + 1)
                    print(json.dumps(sample), flush=True)
                    if run >= 0:
                        result["measurements"].append(sample)
                        save()
        result["summary"] = [{"n": n, "variant": name,
                              "median_ms": statistics.median(s["milliseconds"] for s in result["measurements"]
                                                              if s["n"] == n and s["variant"] == name)}
                             for n in (1, 63, 64, 65, 256, 1024) for name in headers]
        result["status"] = "completed"
    except Exception as error:
        result["error"] = str(error)
        raise
    finally:
        save()


if __name__ == "__main__":
    main()
