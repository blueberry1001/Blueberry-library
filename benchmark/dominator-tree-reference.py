#!/usr/bin/env python3
"""Compare against a cached official Library Checker solution without copying it.

Usage: python3 benchmark/dominator-tree-reference.py path/to/sol/correct.cpp
The original source is Apache-2.0; temporary adaptation keeps only the solver,
uses integer adjacency instead of a one-int Edge, and normalizes root's result.
Both timers include reverse adjacency construction. No reference code is saved.
"""

import hashlib
from pathlib import Path
import resource
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main() -> None:
    print("header_sha256=" + hashlib.sha256((ROOT / "blueberry/graph/dominator-tree.hpp").read_bytes()).hexdigest(), flush=True)
    original = Path(sys.argv[1]).read_bytes()
    print("reference_sha256=" + hashlib.sha256(original).hexdigest(), flush=True)
    print("reference_url=https://github.com/yosupo06/library-checker-problems/blob/master/graph/dominatortree/sol/correct.cpp", flush=True)
    print("Both processes use a 256 MiB stack limit; the reference uses recursive DFS/compression.", flush=True)
    print("Input generation, seed, cases, timing boundaries, and seven repetitions match dominator-tree-benchmark.py.", flush=True)
    source = original.decode().split("int main()", 1)[0]
    replacements = {
        "for (auto e : g[p]) {\n            int d = e.to;": "for (int d : g[p]) {",
        "for (E e : rg[u]) {": "for (int e : rg[u]) {",
        "e.to": "e",
    }
    for old, new in replacements.items():
        assert old in source, old
        source = source.replace(old, new)
    source += """
namespace blueberry {
inline std::vector<int> dominator_tree_vector(const std::vector<std::vector<int>>& graph, int root) {
  if (graph.empty()) { assert(root == -1); return {}; }
  assert(0 <= root && root < static_cast<int>(graph.size()));
  std::vector<std::vector<int>> reverse(graph.size());
  for (int u = 0; u < static_cast<int>(graph.size()); ++u) for (int v : graph[u]) {
    assert(0 <= v && v < static_cast<int>(graph.size()));
    reverse[v].push_back(u);
  }
  auto result = get_dominator(graph, reverse, root);
  result.idom[root] = root;
  return std::move(result.idom);
}
}
"""
    soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
    requested = 256 * 1024 * 1024
    if hard != resource.RLIM_INFINITY:
        requested = min(requested, hard)
    resource.setrlimit(resource.RLIMIT_STACK, (requested, hard))
    print("stack_limit_bytes=" + str(requested), flush=True)
    with tempfile.TemporaryDirectory(prefix="blueberry-dominator-reference-") as directory:
        temp = Path(directory)
        (temp / "dominator-tree-vector.hpp").write_text(source)
        binary = temp / "comparison"
        commands = [
            ["g++", "--version"],
            ["g++", "-std=c++20", "-O2", "-DNDEBUG", "-I", str(ROOT), "-I", str(temp),
             str(ROOT / "benchmark/dominator-tree.cpp"), "-o", str(binary)],
            [str(binary), "compare"],
        ]
        for command in commands:
            print("$ " + " ".join(command), flush=True)
            subprocess.run(command, check=True, cwd=ROOT)
        for mode in ["official", "flat"]:
            memory_log = temp / "memory.txt"
            command = ["/usr/bin/time", "-f", "peak_rss_kib=%M", "-o", str(memory_log), str(binary), mode, "7"]
            print("$ " + " ".join(command), flush=True)
            subprocess.run(command, check=True, cwd=ROOT)
            print(memory_log.read_text().strip(), flush=True)


if __name__ == "__main__":
    main()
