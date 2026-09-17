#!/usr/bin/env python3
"""Compare owned online Li Chao representations and the compressed implementation."""
import hashlib
import json
from pathlib import Path
import platform
import re
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "benchmark/results/dynamic-li-chao-tree-2026-09-17.json"
BUILD = ROOT / ".build/legacy-li-chao/benchmark"
FLAGS = ["-std=gnu++20", "-O2", "-DNDEBUG", "-Wall", "-Wextra", "-Werror", "-I."]
SOURCES = ["benchmark/li-chao-tree.cpp", "benchmark/run_dynamic_li_chao.py",
           "blueberry/data-structure/li-chao-tree.hpp",
           "blueberry/data-structure/dynamic-li-chao-tree.hpp"]


def main():
    BUILD.mkdir(parents=True, exist_ok=True)
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    result = {
        "generated_at_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "revision": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
        "source_sha256": {p: hashlib.sha256((ROOT / p).read_bytes()).hexdigest() for p in SOURCES},
        "compiler": subprocess.check_output(["g++", "--version"], text=True),
        "flags": FLAGS, "platform": platform.platform(),
        "cpu": next(line.split(":", 1)[1].strip() for line in Path("/proc/cpuinfo").read_text().splitlines()
                    if line.startswith("model name")),
        "n": 100000, "operations": 200000, "seed": 1729,
        "repeats": 5, "warmups": 1, "results": [], "succeeded": False,
    }
    # Preserve the independently measured initial header and the iterative-only
    # stage in the result so a fresh checkout can reproduce the ablation run.
    snapshots = {}
    if OUTPUT.exists():
        snapshots = json.loads(OUTPUT.read_text()).get("source_snapshots", {})
    for name in ["recursive", "iterative-midpoint"]:
        path = ROOT / f".build/legacy-li-chao/{name}.hpp"
        if path.exists():
            snapshots[name] = path.read_text()
        if name not in snapshots:
            raise RuntimeError(f"missing measured reference snapshot: {name}")
    result["source_snapshots"] = snapshots
    result["snapshot_sha256"] = {name: hashlib.sha256(source.encode()).hexdigest()
                                 for name, source in snapshots.items()}
    source = (ROOT / "benchmark/li-chao-tree.cpp").read_text()
    includes = []
    aliases = []
    for name, namespace in [("recursive", "blueberry_recursive"),
                            ("iterative-midpoint", "blueberry_iterative")]:
        header = BUILD / (name + ".hpp")
        header.write_text(snapshots[name].replace("namespace blueberry", f"namespace {namespace}"))
        includes.append(f'#include "{header}"')
        aliases.append(f'''class {namespace}Candidate : public {namespace}::DynamicLiChaoTree<Integer> {{
 public:
  explicit {namespace}Candidate(const std::vector<Integer>&)
      : {namespace}::DynamicLiChaoTree<Integer>(domain_low, domain_high) {{}}
}};
''')
    source = "\n".join(includes) + "\n" + source
    source = source.replace("struct Operation {", "\n".join(aliases) + "\nstruct Operation {")
    source = source.replace('candidate != "arena")',
                            'candidate != "arena" && candidate != "arena-recursive" && candidate != "arena-iterative")')
    source = source.replace('else if (candidate == "arena")',
                            'else if (candidate == "arena-recursive") measure<blueberry_recursiveCandidate>(xs, operations, segments);\n'
                            '  else if (candidate == "arena-iterative") measure<blueberry_iterativeCandidate>(xs, operations, segments);\n'
                            '  else if (candidate == "arena")')
    source_path = BUILD / "variants.cpp"
    source_path.write_text(source)
    result["generated_benchmark_sha256"] = hashlib.sha256(source.encode()).hexdigest()
    def save():
        OUTPUT.write_text(json.dumps(result, indent=2) + "\n")
    try:
        executable = BUILD / "li-chao"
        compile_command = ["g++", *FLAGS, str(source_path), "-o", str(executable)]
        compiled = subprocess.run(compile_command, cwd=ROOT, capture_output=True, text=True)
        result["compile"] = {"command": compile_command, "returncode": compiled.returncode,
                             "stdout": compiled.stdout, "stderr": compiled.stderr}
        compiled.check_returncode()
        for workload in ["line", "segment", "sparse-line", "narrow-segment"]:
            checksums = set()
            for candidate in ["compressed", "dynamic", "arena-recursive", "arena-iterative", "arena"]:
                command = ["/usr/bin/time", "-f", "peak_rss_kib=%M", str(executable),
                           candidate, workload, str(result["n"]), str(result["seed"])]
                row = {"candidate": candidate, "workload": workload, "command": command, "runs": []}
                result["results"].append(row)
                for run_index in range(result["warmups"] + result["repeats"]):
                    run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
                    record = {"warmup": run_index == 0, "returncode": run.returncode,
                              "stdout": run.stdout, "stderr": run.stderr}
                    row["runs"].append(record)
                    save()
                    run.check_returncode()
                    fields = dict(re.findall(r"(\w+)=([0-9.]+)", run.stdout + run.stderr))
                    record.update(milliseconds=float(fields["milliseconds"]),
                                  peak_rss_kib=int(fields["peak_rss_kib"]), checksum=fields["checksum"])
                    checksums.add(record["checksum"])
                measured = [r for r in row["runs"] if not r["warmup"]]
                row.update(median_ms=statistics.median(r["milliseconds"] for r in measured),
                           min_ms=min(r["milliseconds"] for r in measured),
                           median_peak_rss_kib=statistics.median(r["peak_rss_kib"] for r in measured))
                save()
            if len(checksums) != 1:
                raise RuntimeError(f"checksum mismatch for {workload}: {checksums}")
        result["succeeded"] = True
    finally:
        save()
    print(OUTPUT)


if __name__ == "__main__":
    main()
