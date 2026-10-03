#!/usr/bin/env python3
"""Compare matching candidates and the local official reference on identical graphs."""

import argparse
import csv
import hashlib
import io
from pathlib import Path
import platform
import shlex
import subprocess


def run(args):
    return subprocess.run(args, check=True, text=True, capture_output=True).stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", type=Path, required=True,
                        help="official graph/general_matching/sol/correct.cpp")
    parser.add_argument("--compilers", nargs="+", default=["g++", "clang++"])
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    build = root / ".benchmark" / "general-matching"
    build.mkdir(parents=True, exist_ok=True)
    header = (root / "blueberry/graph/general-matching.hpp").read_text()
    no_early_stop = header.replace("  if (matching_size == n / 2) return mate;", "")
    no_early_stop = no_early_stop.replace("    if (matching_size == n / 2) break;", "")
    begin = no_early_stop.index("  // A maximal initial matching")
    end = no_early_stop.index("  std::vector<int> parent", begin)
    no_greedy = no_early_stop[:begin] + "  int matching_size = 0;\n\n" + no_early_stop[end:]
    variants = {"no-greedy": no_greedy, "no-early-stop": no_early_stop}
    for name, source in variants.items():
        (build / f"{name}.hpp").write_text(
            source.replace("namespace blueberry", f"namespace benchmark_{name.replace('-', '_')}"))
    reference = args.reference.resolve()
    environment = [
        "Matching benchmark: no input parsing/output formatting inside timed region.",
        "Construction, normalization, matching, result allocation, and cardinality check included.",
        "Each median: seven runs of twenty calls; one untimed warm-up batch.",
        "Graph seed: 20261003. Runs are serial on the same input objects.",
        f"Platform: {platform.platform()}",
        f"Reference: {reference}",
        f"Reference SHA256: {hashlib.sha256(reference.read_bytes()).hexdigest()}",
        f"Header SHA256: {hashlib.sha256(header.encode()).hexdigest()}",
        "Reference adapter applies the same duplicate/self-loop normalization contract.",
        run(["lscpu"]),
    ]
    rows = []
    for compiler in args.compilers:
        environment.append(run([compiler, "--version"]).splitlines()[0])
        for mode in ("assert", "release"):
            flags = ["-std=gnu++20", "-O2"] + (["-DNDEBUG"] if mode == "release" else [])
            binary = build / f"bench-{Path(compiler).name}-{mode}"
            command = [compiler, *flags, "-I", str(root),
                       f'-DGENERAL_MATCHING_NO_GREEDY="{build / "no-greedy.hpp"}"',
                       f'-DGENERAL_MATCHING_NO_EARLY_STOP="{build / "no-early-stop.hpp"}"',
                       f'-DGENERAL_MATCHING_REFERENCE="{reference}"',
                       str(root / "benchmark/general-matching.cpp"), "-o", str(binary)]
            environment.append("Compile: " + shlex.join(command))
            run(command)
            data = run([str(binary)])
            (build / f"{Path(compiler).name}-{mode}.csv").write_text(data)
            for row in csv.DictReader(io.StringIO(data)):
                rows.append({"compiler": compiler, "mode": mode, **row})
            print(f"Completed {compiler} {mode}", flush=True)
    output = root / "benchmark/general-matching-results.csv"
    with output.open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    (root / "benchmark/general-matching-environment.txt").write_text("\n".join(environment))
    print(output)


if __name__ == "__main__":
    main()
