#!/usr/bin/env python3
"""Record compiler output for representative weighted-wavelet operations."""
import argparse
import hashlib
import json
from pathlib import Path
import platform
import re
import subprocess

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument("--header", type=Path, required=True)
parser.add_argument("--out-dir", type=Path, required=True)
args = parser.parse_args()
build = root / ".build/weighted-wavelet-assembly"
(build / "blueberry/data-structure").mkdir(parents=True, exist_ok=True)
header = args.header.read_bytes()
fenwick = (root / "benchmark/weighted-wavelet-rank-fenwick.hpp.txt").read_bytes()
driver = root / "benchmark/weighted-wavelet-assembly.cpp"
(build / "weighted-wavelet-matrix.hpp").write_bytes(header)
(build / "blueberry/data-structure/fenwick-tree.hpp").write_bytes(fenwick)
args.out_dir.mkdir(parents=True, exist_ok=True)
sha = lambda x: hashlib.sha256(x).hexdigest()
meta = {"platform": platform.platform(), "header_sha256": sha(header), "fenwick_sha256": sha(fenwick),
        "driver_sha256": sha(driver.read_bytes()), "builds": []}
for compiler in ("g++", "clang++"):
    for arch in ("portable", "native"):
        flags = ["-std=gnu++20", "-O2", "-DNDEBUG"] + (["-march=native"] if arch == "native" else [])
        target = args.out_dir / (compiler.replace("+", "p") + "-" + arch + ".s")
        cmd = [compiler, *flags, "-I", str(build), "-S", "-masm=intel", str(driver), "-o", str(target)]
        p = subprocess.run(cmd, text=True, capture_output=True)
        row = {"compiler": subprocess.check_output([compiler, "--version"], text=True).splitlines()[0],
               "flags": flags, "exit_code": p.returncode, "stderr": p.stderr, "assembly": target.name}
        if p.returncode == 0:
            asm = target.read_text()
            row["sha256"] = sha(target.read_bytes())
            row["mnemonic_counts_whole_translation_unit"] = {
                "popcnt": len(re.findall(r"(?m)^\s*popcnt\w*\s", asm)),
                "popcount_libcall": len(re.findall(r"(?m)^\s*call\w*\s+[^\n]*__popcountdi2", asm)),
                "integer_division": len(re.findall(r"(?m)^\s*i?div\w*\s", asm)),
                "conditional_branches": len(re.findall(r"(?m)^\s*j(?!mp\b)[a-z]+\s", asm)),
            }
        meta["builds"].append(row)
        (args.out_dir / "metadata.json").write_text(json.dumps(meta, indent=2) + "\n")
        if p.returncode:
            raise RuntimeError(f"assembly generation failed: {compiler} {arch}")
