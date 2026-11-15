#!/usr/bin/env python3
"""Compare historical and current weighted wavelet headers on Windows/MSVC."""
import hashlib
import argparse
import json
import os
from pathlib import Path
import platform
import shutil
import statistics
import subprocess
import time

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument("--output", type=Path, required=True)
args = parser.parse_args()
build = root / ".build/weighted-wavelet-msvc"
build.mkdir(parents=True, exist_ok=True)
vc = Path(r"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.43.34808")
sdk = Path(r"C:\Program Files (x86)\Windows Kits\10")
version = "10.0.22621.0"
compiler = vc / "bin/Hostx64/x64/cl.exe"
env = os.environ.copy()
env["PATH"] = str(compiler.parent) + os.pathsep + env.get("PATH", "")
env["INCLUDE"] = os.pathsep.join(map(str, [vc / "include", *(sdk / "Include" / version / p for p in ("ucrt", "shared", "um"))]))
env["LIB"] = os.pathsep.join(map(str, [vc / "lib/x64", *(sdk / "Lib" / version / p / "x64" for p in ("ucrt", "um"))]))
headers = {"baseline": root / "benchmark/weighted-wavelet-rank-baseline.hpp.txt",
           "measured": root / "benchmark/weighted-wavelet-strided-candidate.hpp.txt",
           "current": root / "blueberry/data-structure/weighted-wavelet-matrix.hpp"}
fenwick = root / "blueberry/data-structure/fenwick-tree.hpp"
driver = root / "benchmark/weighted-wavelet-rank.cpp"
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
result = {"timestamp_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
          "platform": platform.platform(), "processor": platform.processor(),
          "compiler": str(compiler), "flags": ["/std:c++20", "/EHsc", "/O2", "/DNDEBUG"],
          "warmups": 1, "runs": 5, "seed": 1729,
          "sha256": {**{k: sha(p) for k, p in headers.items()}, "fenwick": sha(fenwick), "driver": sha(driver), "runner": sha(Path(__file__))},
          "rss_kib": None, "rss_note": "not measured", "samples": []}
output = args.output
output.parent.mkdir(parents=True, exist_ok=True)
def save():
    output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")

try:
    for name, header in headers.items():
        target = build / name
        include = target / "blueberry/data-structure"
        include.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(header, target / "weighted-wavelet-matrix.hpp")
        shutil.copyfile(fenwick, include / "fenwick-tree.hpp")
        cmd = [str(compiler), "/nologo", *result["flags"], "/I" + str(target), str(driver),
               "/Fo" + str(target / "driver.obj"), "/Fe" + str(target / "driver.exe")]
        p = subprocess.run(cmd, cwd=root, env=env, capture_output=True, text=True, errors="replace")
        if p.returncode:
            result["error"] = {"command": cmd, "returncode": p.returncode, "stdout": p.stdout, "stderr": p.stderr}
            save()
            raise RuntimeError(f"compile failed for {name}")
    cases = [(200000, 20000, s, "int64") for s in ("random", "duplicate", "signed-small", "extrema", "ascending", "biased-head-extreme", "biased-tail-extreme")]
    cases += [(40000, 20000, "random", "string")]
    for n, q, shape, kind in cases:
        checksum = None
        for run in range(-1, result["runs"]):
            names = list(headers)
            offset = (run + 1) % len(names)
            for name in names[offset:] + names[:offset]:
                cmd = [str(build / name / "driver.exe"), str(n), str(q), shape, kind]
                p = subprocess.run(cmd, cwd=root, env=env, capture_output=True, text=True, errors="replace")
                if p.returncode:
                    result["error"] = {"command": cmd, "returncode": p.returncode, "stdout": p.stdout, "stderr": p.stderr}
                    save()
                    raise RuntimeError(f"execution failed for {name}")
                fields = p.stdout.split()
                if len(fields) != 3 or (checksum is not None and fields[0] != checksum):
                    result["error"] = {"command": cmd, "stdout": p.stdout, "expected_checksum": checksum}
                    save()
                    raise RuntimeError("invalid output or checksum mismatch")
                checksum = fields[0]
                if run >= 0:
                    result["samples"].append({"n": n, "q": q, "shape": shape, "type": kind, "run": run + 1,
                                              "variant": name, "checksum": checksum,
                                              "build_ms": float(fields[1]), "operations_ms": float(fields[2])})
                    save()
    result["status"] = "complete"
    save()
    for n, q, shape, kind in cases:
        row = []
        for name in headers:
            samples = [x for x in result["samples"] if x["shape"] == shape and x["type"] == kind and x["variant"] == name]
            row.append(f"{name}={statistics.median(x['build_ms'] for x in samples):.2f}/{statistics.median(x['operations_ms'] for x in samples):.2f}")
        print(shape, kind, " ".join(row), flush=True)
except BaseException as exc:
    result.setdefault("error", {"exception": repr(exc)})
    result["status"] = "failed"
    save()
    raise
