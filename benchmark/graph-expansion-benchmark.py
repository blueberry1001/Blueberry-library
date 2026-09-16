#!/usr/bin/env python3
"""Reproducible graph layout comparison; generates variants of our own code only."""
from pathlib import Path
import argparse
import csv
import hashlib
import json
import os
import platform
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--compiler", default="g++")
parser.add_argument("--output", type=Path, default=ROOT / "benchmark/results/lc-expansion-graph")
args = parser.parse_args()
args.output.mkdir(parents=True, exist_ok=True)
build = ROOT / ".build/lc-expansion-graph/benchmark"
build.mkdir(parents=True, exist_ok=True)

def replace(text, old, new):
    assert old in text, f"Update benchmark variant generator: {old!r}"
    return text.replace(old, new)

bcc = (ROOT / "blueberry/graph/biconnected-components.hpp").read_text()
begin = bcc.index("    std::vector<std::size_t> offset")
end = bcc.index("    next = offset;", begin) + len("    next = offset;")
bcc = bcc[:begin] + """    std::vector<std::vector<int>> adjacency(n);
    for (int id = 0; id < static_cast<int>(edges.size()); ++id) {
      const auto [u, v] = edges[id];
      assert(0 <= u && u < n && 0 <= v && v < n && u != v);
      adjacency[u].push_back(id); adjacency[v].push_back(id);
    }
    std::vector<std::size_t> next(n);""" + bcc[end:]
bcc = replace(bcc, "BiconnectedComponents", "NestedBiconnectedComponents")
bcc = replace(bcc, "offset[root] == offset[root + 1]", "adjacency[root].empty()")
bcc = replace(bcc, "offset[v + 1]", "adjacency[v].size()")
bcc = replace(bcc, "adjacency[next[v]++]", "adjacency[v][next[v]++]")
(build / "nested-biconnected-components.hpp").write_text(bcc)

euler = (ROOT / "blueberry/graph/eulerian-trail.hpp").read_text()
begin = euler.index("    for (int v = 0; v < n; ++v) offset[v + 1] += offset[v];")
end = euler.index("    next = offset;", begin) + len("    next = offset;")
euler = euler[:begin] + """    std::vector<std::vector<int>> adjacency(n);
    for (int id = 0; id < m; ++id) {
      const auto [u, v] = edges[id];
      adjacency[u].push_back(id);
      if constexpr (!Directed) adjacency[v].push_back(id);
    }
    std::vector<std::size_t> next(n);""" + euler[end:]
euler = replace(euler, "EulerianTrail", "NestedEulerianTrail")
euler = replace(euler, "next[v] < offset[v + 1]", "next[v] < adjacency[v].size()")
euler = replace(euler, "adjacency[next[v]]", "adjacency[v][next[v]]")
euler = replace(euler, "adjacency[next[v]++]", "adjacency[v][next[v]++]")
(build / "nested-eulerian-trail.hpp").write_text(euler)

command = [args.compiler, "-std=c++20", "-O2", "-DNDEBUG", "-Wall", "-Wextra", "-Werror", "-I", str(ROOT), "-isystem", str(ROOT / ".deps/ac-library"), "-I", str(build), str(ROOT / "benchmark/graph-expansion.cpp"), "-o", str(build / "benchmark")]
sources = [ROOT / "benchmark/graph-expansion.cpp", Path(__file__),
           *(ROOT / "blueberry/graph" / (name + ".hpp") for name in ("hopcroft-karp", "biconnected-components", "eulerian-trail")),
           build / "nested-biconnected-components.hpp", build / "nested-eulerian-trail.hpp"]
cpu = platform.processor()
if Path("/proc/cpuinfo").exists():
    cpu = next((line.split(":", 1)[1].strip() for line in Path("/proc/cpuinfo").read_text().splitlines() if line.startswith("model name")), cpu)
metadata = {"compiler": subprocess.check_output([args.compiler, "--version"], text=True).splitlines()[0],
            "command": command, "platform": platform.platform(), "cpu": cpu,
            "seed": 20260917, "runs": 5, "warmups": 1,
            "source_sha256": {str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest() for path in sources}}
label = Path(args.compiler).name
(args.output / (label + "-environment.json")).write_text(json.dumps(metadata, indent=2) + "\n")
with (args.output / (label + "-compile.log")).open("w") as log:
    subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, check=True)
with (args.output / (label + "-runs.csv")).open("w") as log, (args.output / (label + "-stderr.log")).open("w") as errors:
    subprocess.run([str(build / "benchmark")], stdout=log, stderr=errors, check=True, timeout=120)
rows = list(csv.DictReader((args.output / (label + "-runs.csv")).open()))
summary = []
for shape, candidate in sorted({(row["shape"], row["candidate"]) for row in rows}):
    samples = [float(row["seconds"]) for row in rows if row["shape"] == shape and row["candidate"] == candidate]
    summary.append({"shape": shape, "candidate": candidate, "median_seconds": statistics.median(samples), "min_seconds": min(samples)})
(args.output / (label + "-summary.json")).write_text(json.dumps(summary, indent=2) + "\n")
print(json.dumps(summary, indent=2))
