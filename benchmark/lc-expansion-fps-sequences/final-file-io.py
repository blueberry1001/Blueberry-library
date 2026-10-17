#!/usr/bin/env python3
"""Interleave identical full-output I/O drivers and use real file stdin."""
import json
from pathlib import Path
import subprocess
import tempfile
import time

root = Path(__file__).resolve().parents[2]
out = Path(__file__).resolve().parent
common = ["g++", "-std=gnu++20", "-O2", "-DNDEBUG", "-I.", "-isystem", ".deps/ac-library"]
for mode in ["iostream", "fastio"]:
    command = common + (["-DUSE_FAST_IO"] if mode == "fastio" else [])
    command += [str(out / "io-driver.cpp"), "-o", str(root / f".build/fps-sequences/io-{mode}")]
    subprocess.run(command, check=True)

rows = []
for n in [8192, 131072, 500000]:
    expected = {}
    with tempfile.TemporaryFile(mode="w+") as input_file:
        input_file.write(str(n) + "\n")
        input_file.flush()
        for repeat in range(3):
            cases = [(name, mode, None) for name in ["bernoulli", "bell", "partition"]
                     for mode in ["iostream", "fastio"]] + [("partition", "external", 207699)]
            for name, mode, submission in cases:
                binary = root / f".build/fps-sequences/io-{mode}"
                command = [str(binary), name]
                if submission:
                    command = [str(root / f".build/fps-sequences/fastest-{submission}")]
                input_file.seek(0)
                begin = time.perf_counter()
                run = subprocess.run(["/usr/bin/time", "-f", "%e,%M"] + command,
                                     stdin=input_file, text=True, capture_output=True)
                row = dict(n=n, repeat=repeat, name=name, mode=mode, submission=submission,
                           returncode=run.returncode, process_ms=(time.perf_counter() - begin) * 1000,
                           time_stderr=run.stderr)
                if run.returncode == 0:
                    values = [int(s) for s in run.stdout.split()]
                    if name not in expected:
                        expected[name] = values
                    row["matches"] = len(values) == n + 1 and values == expected[name]
                    row["checksum"] = sum((i + 1) * v for i, v in enumerate(values)) % (1 << 64)
                    assert row["matches"], row
                rows.append(row)
                (out / "final-file-io.json").write_text(json.dumps({
                    "input": "regular file; same input file for every driver",
                    "flags": common, "samples": rows}, indent=2) + "\n")
