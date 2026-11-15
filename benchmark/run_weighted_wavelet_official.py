#!/usr/bin/env python3
"""Compare local implementations against a separately downloaded LC submission."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import subprocess
import time

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument("--external", type=Path, required=True)
parser.add_argument("--candidate", type=Path, required=True)
parser.add_argument("--cases", type=Path, required=True)
parser.add_argument("--output", type=Path, required=True)
parser.add_argument("--compiler", default="g++")
parser.add_argument("--runs", type=int, default=3)
args = parser.parse_args()
build = root / ".build/weighted-wavelet-official"
build.mkdir(parents=True, exist_ok=True)
revision = "efa8b4e97ce68395657c643fd6c04f2fba56b826"
flags = ["-std=gnu++20", "-O2", "-DNDEBUG"]
baseline = (root / "benchmark/weighted-wavelet-rank-baseline.hpp.txt").read_bytes()
fenwick = (root / "benchmark/weighted-wavelet-rank-fenwick.hpp.txt").read_bytes()
fastio = subprocess.check_output(["git", "-C", str(root), "show", revision + ":blueberry/utility/fast-io.hpp"])
driver = root / "benchmark/weighted-wavelet-official.cpp"
headers = {"baseline": baseline, "candidate": args.candidate.read_bytes()}
for name, source in headers.items():
    inc = build / name / "blueberry"
    (inc / "data-structure").mkdir(parents=True, exist_ok=True)
    (inc / "utility").mkdir(exist_ok=True)
    (inc / "data-structure/weighted-wavelet-matrix.hpp").write_bytes(source)
    (inc / "data-structure/fenwick-tree.hpp").write_bytes(fenwick)
    (inc / "utility/fast-io.hpp").write_bytes(fastio)
    subprocess.run([args.compiler, *flags, "-I", str(build / name), str(driver), "-o", str(build / (name + ".out"))], check=True)
subprocess.run([args.compiler, *flags, str(args.external), "-o", str(build / "external.out")], check=True)
sha = lambda x: hashlib.sha256(x).hexdigest()
cpu = next((s.split(":", 1)[1].strip() for s in Path("/proc/cpuinfo").read_text().splitlines() if s.startswith("model name")), platform.processor())
data = {"revision": revision, "timestamp_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "cpu": cpu, "platform": platform.platform(), "compiler": subprocess.check_output([args.compiler, "--version"], text=True).splitlines()[0],
        "flags": flags, "runs": args.runs, "sha256": {**{k: sha(v) for k,v in headers.items()}, "fenwick": sha(fenwick), "fastio": sha(fastio),
        "driver": sha(driver.read_bytes()), "external": sha(args.external.read_bytes()), "runner": sha(Path(__file__).read_bytes())}, "samples": []}
args.output.parent.mkdir(parents=True, exist_ok=True)
for case in ("max_random_00", "xy_concentrate_00", "many_points_00", "many_queries_00"):
    inp, expected = args.cases / "in" / (case + ".in"), args.cases / "out" / (case + ".out")
    expected_bytes = expected.read_bytes()
    case_sha, expected_sha = sha(inp.read_bytes()), sha(expected_bytes)
    variants = list(headers) + ["external"]
    for run in range(-1, args.runs):
        offset = (run + 1) % len(variants)
        for name in variants[offset:] + variants[:offset]:
            outfile, metrics = build / "actual.out", build / "time.txt"
            cmd = ["/usr/bin/time", "-f", "%M", "-o", str(metrics), str(build / (name + ".out"))]
            with inp.open("rb") as source, outfile.open("wb") as target:
                start = time.perf_counter()
                p = subprocess.run(cmd, stdin=source, stdout=target, stderr=subprocess.PIPE)
                elapsed = time.perf_counter() - start
            actual = outfile.read_bytes()
            if p.returncode or actual != expected_bytes:
                data["error"] = {"case": case, "variant": name, "returncode": p.returncode,
                    "stderr": p.stderr.decode(errors="replace"), "actual_sha256": sha(actual), "expected_sha256": expected_sha}
                args.output.write_text(json.dumps(data, indent=2) + "\n")
                raise RuntimeError("execution failure or output mismatch")
            print(case, run, name, round(elapsed * 1000, 3), flush=True)
            if run >= 0:
                data["samples"].append({"case": case, "run": run + 1, "variant": name, "milliseconds": elapsed * 1000,
                    "rss_kib": int(metrics.read_text().strip()), "input_sha256": case_sha, "output_sha256": expected_sha})
                args.output.write_text(json.dumps(data, indent=2) + "\n")
