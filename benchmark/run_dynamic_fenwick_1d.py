#!/usr/bin/env python3
"""Compare ABC484 E-like online 1D prefix-count workloads.

Provide paths to a frozen pre-change header and the downloaded Nyaan headers.
Each variant runs in a separate process; input generation is outside the timer.
"""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import subprocess
import time

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
baseline = parser.add_mutually_exclusive_group(required=True)
baseline.add_argument("--baseline", type=Path)
baseline.add_argument("--no-baseline", action="store_true", help="compare candidate and Nyaan only")
parser.add_argument("--candidate", type=Path, default=root / "blueberry/data-structure/dynamic-fenwick-tree.hpp")
parser.add_argument("--compare", type=Path, help="optional second candidate header")
parser.add_argument("--driver", type=Path, default=root / "benchmark/dynamic-fenwick-1d.cpp")
parser.add_argument("--nyaan", type=Path, required=True, help="directory containing both Nyaan headers")
parser.add_argument("--compiler", default="g++")
parser.add_argument("--count", type=int, default=300000)
parser.add_argument("--runs", type=int, default=5)
parser.add_argument("--shapes", nargs="+", default=["walk", "diffuse", "wide", "ascending", "contest-near", "contest-diffuse", "contest-broad"])
parser.add_argument("--asserts", action="store_true", help="leave assertions enabled")
parser.add_argument("--output", type=Path, required=True)
args = parser.parse_args()
build = root / ".build/dynamic-fenwick-1d"
build.mkdir(parents=True, exist_ok=True)
driver = args.driver.resolve()
candidate = args.candidate.resolve()
flags = ["-std=gnu++20", "-O2"] + ([] if args.asserts else ["-DNDEBUG"])
headers = {"candidate": candidate, "nyaan": args.nyaan.resolve() / "dynamic-binary-indexed-tree.hpp"}
if args.baseline:
    headers = {"baseline": args.baseline.resolve(), **headers}
if args.compare:
    headers["compare"] = args.compare.resolve()

for name, path in headers.items():
    # A private include directory freezes the selected header for this compile.
    include = build / name
    include.mkdir(exist_ok=True)
    if name != "nyaan":
        (include / "dynamic-fenwick-tree.hpp").write_bytes(path.read_bytes())
    else:
        for filename in ("dynamic-binary-indexed-tree.hpp", "hash-map-variable-length.hpp"):
            (include / filename).write_bytes((args.nyaan / filename).read_bytes())
    command = [args.compiler, *flags, "-I", str(include), str(driver), "-o", str(build / (name + ".out"))]
    if name == "nyaan":
        command.insert(1, "-DNYAAN")
    subprocess.run(command, check=True)

cpu = next((line.split(":", 1)[1].strip() for line in Path("/proc/cpuinfo").read_text().splitlines() if line.startswith("model name")), platform.processor())
result = {"timestamp_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
          "platform": platform.platform(), "cpu": cpu,
          "compiler": subprocess.check_output([args.compiler, "--version"], text=True).splitlines()[0],
          "flags": flags, "count": args.count, "runs": args.runs, "seed": 1729,
          "sha256": {name: hashlib.sha256(path.read_bytes()).hexdigest() for name, path in {**headers, "driver": driver, "nyaan_hash": args.nyaan / "hash-map-variable-length.hpp"}.items()},
          "measurements": []}

for shape in args.shapes:
    expected = None
    # Rotate order to reduce systematic thermal/load bias. First round warms up.
    variants = list(headers)
    for run in range(-1, args.runs):
        offset = (run + 1) % len(variants)
        order = variants[offset:] + variants[:offset]
        for name in order:
            cmd = ["/usr/bin/time", "-f", "%M", str(build / (name + ".out")), str(args.count), shape]
            process = subprocess.run(cmd, text=True, capture_output=True, check=True)
            _, _, checksum, elapsed = process.stdout.split()
            if expected is None:
                expected = checksum
            if checksum != expected:
                raise RuntimeError(f"checksum mismatch for {shape}: {name} {checksum} != {expected}")
            if run >= 0:
                result["measurements"].append({"shape": shape, "run": run + 1, "variant": name,
                    "milliseconds": float(elapsed), "peak_rss_kib": int(process.stderr.strip().splitlines()[-1]), "checksum": int(checksum)})
            print(shape, run, name, elapsed, "ms", flush=True)
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text(json.dumps(result, indent=2) + "\n")
