"""Summarize only matching per-run conditions; retain all candidate/failure logs."""
from pathlib import Path
import csv
import hashlib
import json
import statistics
ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT/"benchmark/results/contest-template-import"
lines = ["# Contest template import measurements", "", "5 runs, fixed seed12345, WSL2; see environment.json and validation.json for machine/compiler/flags.",
         "Allocation+construction and pre-generated query loop are timed separately. No parse/format time is included in table timings.",
         "Shared desktop host timings are descriptive, especially small differences; each source/flag/input set is kept separate.", "",
         "| compiler/revision | variant | build median / min (ms) | query median / min (ms) |", "| --- | --- | ---: | ---: |"]
summaries = {}
for compiler in ["g++", "clang++"]:
    for suffix in ["-before-raw", "-raw-candidate", ""]:
        path = OUT/f"{compiler}{suffix}.csv"
        grouped = {}
        for row in csv.DictReader(path.open()): grouped.setdefault(row["variant"], []).append(row)
        for name, rows in grouped.items():
            builds = [float(row["build_seconds"])*1000 for row in rows]
            queries = [float(row["query_seconds"])*1000 for row in rows]
            assert len(rows) == 5
            assert len({row["checksum"] for row in rows}) == 1
            key = compiler+(suffix or "-final")+"/"+name
            stats = dict(build_median_ms=statistics.median(builds), build_min_ms=min(builds),
                         query_median_ms=statistics.median(queries), query_min_ms=min(queries))
            summaries[key] = stats
            lines.append(f"| {compiler}{suffix or '-final'} | {name} | {stats['build_median_ms']:.3f} / {stats['build_min_ms']:.3f} | {stats['query_median_ms']:.3f} / {stats['query_min_ms']:.3f} |")
lines.extend(["", "Same release flags / official maximum case; 3 runs with official checker after each run:", "",
              "| driver revision | variant | median / min (ms) | peak RSS median (KiB) |", "| --- | --- | ---: | ---: |"])
for name in ["fastest-local-before-driver-io.json", "fastest-local.json"]:
    data = json.loads((OUT/name).read_text())
    for variant in ["fastest-223248", "blueberry-driver"]:
        rows = [r for r in data["results"] if r["variant"] == variant]
        assert all(r["status"] == r["checker_status"] == 0 for r in rows)
        times = [r["seconds"]*1000 for r in rows]
        memory = [int(r["peak_rss_kib"]) for r in rows]
        lines.append(f"| {name} | {variant} | {statistics.median(times):.3f} / {min(times):.3f} | {statistics.median(memory):.0f} |")
lines.extend(["", "The final driver retains query storage (roughly 8 MB for T=1e6) to size the generic table to actual maximum n.",
              "The fastest source has AVX2 target pragmas and a fixed common-prime table bound. Its public code is research-only and is not redistributed.",
              "The initial unmatched assert-flags experiment and ACL multi-TU failure are preserved but excluded from the performance comparison.", ""])
snapshots = {p: hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in ["blueberry/math/combinations.hpp", "blueberry/utility/contest-helpers.hpp", "blueberry/utility/contest-template.hpp", "verify/math/binomial-coefficient-prime-mod.test.cpp"]}
(OUT/"report.md").write_text("\n".join(lines))
(OUT/"summary.json").write_text(json.dumps(dict(summaries=summaries, final_source_sha256=snapshots), indent=2)+"\n")
print("Saved matching-condition medians and final header hashes")
