#!/usr/bin/env python3
"""Supplement immutable HLD measurements with paired-ratio sensitivity checks."""
import argparse
import csv
import math
from pathlib import Path
import statistics

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("runs", nargs="*", default=["candidate1", "candidate2-clean"])
args = parser.parse_args()
root = Path(__file__).resolve().parent / "results/hld"
for name in args.runs:
    if Path(name).name != name:
        parser.error("run names must be single directory names")
    groups = {}
    with (root / name / "raw.csv").open() as stream:
        for row in csv.DictReader(stream):
            key = tuple(row[k] for k in ["compiler", "mode", "shape", "operation"])
            sample = groups.setdefault(key, {}).setdefault(int(row["repeat"]), {})
            if row["variant"] in sample or not math.isfinite(float(row["ms"])) or float(row["ms"]) <= 0:
                raise ValueError("duplicate, missing, or nonpositive timing")
            sample[row["variant"]] = row
    result = []
    for key, samples in sorted(groups.items()):
        before, after, ratios = [], [], []
        for pair in samples.values():
            if pair.keys() != {"baseline", "candidate"}:
                raise ValueError("incomplete before/after pair")
            b, a = pair["baseline"], pair["candidate"]
            if any(b[k] != a[k] for k in ["n", "q", "seed", "checksum", "calls"]):
                raise ValueError("paired input or result mismatch")
            before.append(float(b["ms"]))
            after.append(float(a["ms"]))
            ratios.append(float(a["ms"]) / float(b["ms"]))
        result.append(dict(zip(["compiler", "mode", "shape", "operation"], key),
                           repeats=len(samples),
                           ratio_of_medians=statistics.median(after) / statistics.median(before),
                           median_paired_ratio=statistics.median(ratios),
                           min_paired_ratio=min(ratios), max_paired_ratio=max(ratios)))
    with (root / name / "paired-ratios.csv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(result[0]), lineterminator="\n")
        writer.writeheader()
        writer.writerows(result)
    print(f"{name}: {len(result)} groups, complete matching before/after pairs")
