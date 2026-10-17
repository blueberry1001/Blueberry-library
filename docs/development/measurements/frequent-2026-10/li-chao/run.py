"""Compare each optimization separately with immutable baseline d6af3e9."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import statistics
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[4]
BASE = "d6af3e9"
STATIC = "blueberry/data-structure/li-chao-tree.hpp"
DYNAMIC = "blueberry/data-structure/dynamic-li-chao-tree.hpp"


def baseline(path):
    return subprocess.check_output(["git", "show", f"{BASE}:{path}"], cwd=ROOT, text=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--assertions", action="store_true")
    parser.add_argument("--repeats", type=int, default=5)
    parser.add_argument("--n", type=int, default=100000)
    parser.add_argument("--prepare-only", action="store_true")
    parser.add_argument("--query-variant", choices=["bottom-up", "top-down"], default="bottom-up")
    args = parser.parse_args()
    flags = ["-std=gnu++20", "-O2", "-Wall", "-Wextra", "-Werror"]
    if not args.assertions:
        flags += ["-DNDEBUG"]
    result = {"baseline": BASE, "compiler": subprocess.check_output([args.compiler, "--version"], text=True),
              "platform": platform.platform(), "cpu": next((x.split(":", 1)[1].strip() for x in
              Path("/proc/cpuinfo").read_text().splitlines() if x.startswith("model name")), "unknown"),
              "flags": flags, "n": args.n, "seed": 1729, "repeats": args.repeats, "warmups": 1,
              "cpu_affinity": sorted(os.sched_getaffinity(0)),
              "benchmark_sha256": hashlib.sha256((HERE / "benchmark.cpp").read_bytes()).hexdigest(),
              "query_variant": args.query_variant, "results": []}
    suffix = "-top-down" if args.query_variant == "top-down" else ""
    output = HERE / (Path(args.compiler).name + ("-assert" if args.assertions else "-release") + suffix + ".json")
    def save():
        output.write_text(json.dumps(result, indent=2) + "\n")
    with tempfile.TemporaryDirectory(prefix="li-chao-frequent-") as temp:
        temp = Path(temp)
        old_static, old_dynamic = baseline(STATIC), baseline(DYNAMIC)
        new_static, new_dynamic = (ROOT / STATIC).read_text(), (ROOT / DYNAMIC).read_text()
        # Isolate the query and segment edits by replacing only the complete query method.
        def query(source):
            return source[source.index("  std::optional<T> query(T x) const {"):source.index("\n private:")]
        if args.query_variant == "bottom-up":
            new_static = new_static.replace(query(new_static), (HERE / "bottom-up-query.txt").read_text().rstrip() + "\n")
        query_static = old_static.replace(query(old_static), query(new_static))
        prune_static = new_static.replace(query(new_static), query(old_static))
        sources = [("baseline-static", old_static, "baseline"), ("query-static", query_static, "queryonly"),
                   ("prune-static", prune_static, "pruneonly"), ("combined-static", new_static, "combined"),
                   ("baseline-dynamic", old_dynamic, "baseline"), ("prune-dynamic", new_dynamic, "prunedynamic")]
        result["source_sha256"] = {}
        for name, source, namespace in sources:
            source = source.replace("namespace blueberry", "namespace " + namespace)
            (temp / (name + ".hpp")).write_text(source)
            result["source_sha256"][name] = hashlib.sha256(source.encode()).hexdigest()
        executable = temp / "benchmark"
        subprocess.run([args.compiler, *flags, "-I" + str(temp), str(HERE / "benchmark.cpp"), "-o", str(executable)], check=True)
        if args.prepare_only:
            print("compile OK", args.compiler, flags)
            return
        workloads = ["envelope-line", "random-line", "query-heavy-line", "duplicate-line", "ordered-line",
                     "envelope-segment", "random-segment", "narrow-segment"]
        for workload in workloads:
            checksums = set()
            for name, _, _ in sources:
                command = ["/usr/bin/time", "-f", "peak_rss_kib=%M", str(executable), name, workload, str(args.n), "1729"]
                subprocess.run(command, capture_output=True, check=True)
                row = {"candidate": name, "workload": workload, "runs": []}
                result["results"].append(row)
                for iteration in range(args.repeats):
                    run = subprocess.run(command, capture_output=True, text=True)
                    entry = {"iteration": iteration, "returncode": run.returncode, "stdout": run.stdout, "stderr": run.stderr}
                    row["runs"].append(entry)
                    save()
                    if run.returncode:
                        raise RuntimeError(entry)
                    fields = dict(re.findall(r"(\w+)=([0-9.]+)", run.stdout + run.stderr))
                    entry.update({key: float(value) if key.endswith("_ms") else int(value) for key, value in fields.items()})
                    checksums.add((fields["checksum"], fields["batch_checksum"]))
                row["median"] = {key: statistics.median(run[key] for run in row["runs"]) for key in
                                 ["construction_ms", "interleaved_ms", "updates_ms", "queries_ms", "peak_rss_kib"]}
                row["min"] = {key: min(run[key] for run in row["runs"]) for key in ["construction_ms", "interleaved_ms", "updates_ms", "queries_ms"]}
                save()
                print(workload, name, row["median"], flush=True)
            if len(checksums) != 1:
                raise RuntimeError((workload, checksums))
        result["checksums_match"] = True
        save()
    print(output)


if __name__ == "__main__":
    main()
