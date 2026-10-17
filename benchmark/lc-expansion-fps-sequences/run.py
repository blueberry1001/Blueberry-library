#!/usr/bin/env python3
"""Run isolated, same-contract sequence candidates and retain every sample."""
import csv
import json
from pathlib import Path
import platform
import statistics
import subprocess
import time

root = Path(__file__).resolve().parents[2]
out = Path(__file__).resolve().parent
binary = root / ".build/fps-sequences/compare"
modes = ["bernoulli", "bernoulli_repeated_inverse", "bell", "bell_two_exp",
         "partition", "partition_pentagonal_dp", "partition_sigma_exp", "io_stream", "io_chars"]
rows = []
for n in [8192, 131072, 500000]:
    for repeat in range(3):
        for mode in modes:
            before = time.perf_counter()
            process = subprocess.run(["/usr/bin/time", "-f", "%M", str(binary), mode, str(n)],
                                     check=True, text=True, capture_output=True)
            fields = process.stdout.strip().split(",")
            rows.append(dict(mode=mode, n=n, repeat=repeat,
                             core_ms=float(fields[2]), query_ms=float(fields[3]),
                             format_ms=float(fields[4]), parse_ms=float(fields[5]),
                             checksum=int(fields[6]), parsed_sum=int(fields[7]),
                             peak_rss_kib=int(process.stderr.strip()),
                             process_ms=(time.perf_counter() - before) * 1000))
            (out / "raw.json").write_text(json.dumps(rows, indent=2) + "\n")
    for group in [["bernoulli", "bernoulli_repeated_inverse"], ["bell", "bell_two_exp"],
                  ["partition", "partition_pentagonal_dp", "partition_sigma_exp"], ["io_stream", "io_chars"]]:
        assert len({row["checksum"] for row in rows if row["n"] == n and row["mode"] in group}) == 1
with (out / "summary.csv").open("w") as stream:
    writer = csv.writer(stream)
    writer.writerow(["mode", "n", "runs", "median_core_ms", "min_core_ms", "median_query_ms",
                     "median_format_ms", "median_parse_ms", "median_process_ms", "peak_rss_kib"])
    for n in [8192, 131072, 500000]:
        for mode in modes:
            group = [row for row in rows if row["n"] == n and row["mode"] == mode]
            median = lambda key: statistics.median(row[key] for row in group)
            writer.writerow([mode, n, len(group), median("core_ms"), min(row["core_ms"] for row in group),
                             median("query_ms"), median("format_ms"), median("parse_ms"),
                             median("process_ms"), max(row["peak_rss_kib"] for row in group)])
(out / "environment.json").write_text(json.dumps({
    "platform": platform.platform(), "cpu": next(line.split(":", 1)[1].strip() for line in
    Path("/proc/cpuinfo").read_text().splitlines() if line.startswith("model name")),
    "compiler": subprocess.check_output(["g++", "--version"], text=True),
    "flags": "-std=gnu++20 -O2 -DNDEBUG -I. -isystem .deps/ac-library",
    "revision": subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip(),
    "runs": 3, "io": "format/parse same coefficients in memory; actual stdout is metrics only",
}, indent=2) + "\n")
