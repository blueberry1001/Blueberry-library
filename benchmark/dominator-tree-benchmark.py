#!/usr/bin/env python3
"""Compare the same iterative LT implementation with two storage layouts.

Run from the repository root. Generated headers/binaries stay in a temporary
directory. Raw measurements and environment are printed to stdout as a log.
"""

from pathlib import Path
import hashlib
import platform
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]


def vector_variant() -> str:
    source = (ROOT / "blueberry/graph/dominator-tree.hpp").read_text()
    source = source.replace("dominator_tree(", "dominator_tree_vector(", 1)
    start = source.index("  // Flat reverse adjacency")
    end = source.index("  std::vector<int> semi", start)
    source = source[:start] + """  std::vector<std::vector<int>> predecessor(reached);
  for (int v = 0; v < n; ++v) for (int to : graph[v]) {
    assert(0 <= to && to < n);
    if (index[v] != -1) predecessor[index[to]].push_back(index[v]);
  }

""" + source[end:]
    replacements = {
        "  std::vector<int> idom(reached, -1), bucket(reached, -1), next_bucket(reached, -1);":
        "  std::vector<int> idom(reached, -1);\n  std::vector<std::vector<int>> bucket(reached);",
        "    for (std::size_t i = offset[w]; i < offset[w + 1]; ++i)\n      semi[w] = std::min(semi[w], semi[eval(predecessor[i])]);":
        "    for (int v : predecessor[w]) semi[w] = std::min(semi[w], semi[eval(v)]);",
        "    next_bucket[w] = bucket[semi[w]];\n    bucket[semi[w]] = w;":
        "    bucket[semi[w]].push_back(w);",
        "    for (int v = bucket[p]; v != -1; v = next_bucket[v]) {":
        "    for (int v : bucket[p]) {",
        "    bucket[p] = -1;": "    bucket[p].clear();",
    }
    for old, new in replacements.items():
        assert source.count(old) == 1, old
        source = source.replace(old, new)
    return source


def run(command: list[str]) -> None:
    print("$ " + " ".join(command), flush=True)
    subprocess.run(command, check=True, cwd=ROOT)


def main() -> None:
    print("header_sha256=" + hashlib.sha256((ROOT / "blueberry/graph/dominator-tree.hpp").read_bytes()).hexdigest(), flush=True)
    print("platform:", platform.platform(), flush=True)
    run(["uname", "-a"])
    run(["lscpu"])
    print("Seed=20261003; one warmup, seven measured runs per case; no I/O inside solve timer.", flush=True)
    print("solve_ms includes reverse adjacency, LT, output allocation and temporary destruction.", flush=True)
    print("input_build_ms is caller-owned adjacency generation; RSS includes input/output and runtime.", flush=True)
    with tempfile.TemporaryDirectory(prefix="blueberry-dominator-benchmark-") as directory:
        temp = Path(directory)
        (temp / "dominator-tree-vector.hpp").write_text(vector_variant())
        for compiler in ["g++", "clang++"]:
            run([compiler, "--version"])
            for release in [False, True]:
                binary = temp / (compiler + ("-release" if release else "-assert"))
                flags = ["-std=c++20", "-O2"] + (["-DNDEBUG"] if release else [])
                run([compiler, *flags, "-I", str(ROOT), "-I", str(temp),
                     str(ROOT / "benchmark/dominator-tree.cpp"), "-o", str(binary)])
                run([str(binary), "compare"])
                # Each layout gets the same generated input and the same compiler settings.
                for layout in ["vectors", "flat"]:
                    memory_log = temp / "memory.txt"
                    run(["/usr/bin/time", "-f", "peak_rss_kib=%M", "-o", str(memory_log),
                         str(binary), layout, "7"])
                    print(memory_log.read_text().strip(), flush=True)


if __name__ == "__main__":
    main()
