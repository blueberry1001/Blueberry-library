#!/usr/bin/env python3
"""Derive comparison CSV/report from retained furthest-pair resume measurements."""
import argparse
import csv
import json
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[1]
BASE = ROOT / "benchmark/results/furthest-pair-resume"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--experiment", default="initial")
    args = parser.parse_args()
    folder = BASE / args.experiment
    records = json.loads((folder / "measurement/summary.json").read_text())
    metadata = json.loads((folder / "measurement/metadata.json").read_text())
    prepared = json.loads((folder / "prepared.json").read_text())
    comparisons = []
    for candidate in records:
        if candidate["variant"] == "blueberry":
            continue
        keys = ["family", "compiler", "mode", "workload"] + (["size"] if candidate["family"] == "kernel" else ["io"])
        baseline = next(row for row in records if row["variant"] == "blueberry" and all(row[k] == candidate[k] for k in keys))
        for stage, result in candidate["summary"].items():
            if stage not in baseline["summary"]:
                continue
            before = baseline["summary"][stage]
            comparisons.append(dict(family=candidate["family"], compiler=candidate["compiler"], mode=candidate["mode"],
                workload=candidate["workload"], size=candidate.get("size", ""), io=candidate.get("io", ""),
                variant=candidate["variant"], stage=stage, baseline_median_ns=before["median"],
                candidate_median_ns=result["median"], baseline_min_ns=before["min"], baseline_max_ns=before["max"],
                candidate_min_ns=result["min"], candidate_max_ns=result["max"],
                delta_percent=100 * (result["median"] / before["median"] - 1),
                ranges_separated_faster=result["max"] < before["min"],
                ranges_separated_slower=result["min"] > before["max"]))
    with (folder / "stage-comparisons.csv").open("w", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(comparisons[0]), lineterminator="\n")
        writer.writeheader(); writer.writerows(comparisons)
    lines = ["# furthest_pair: initial comparison results", "",
        "This report derives only this experiment's retained raw measurements. It does not declare production adoption or final validation.", "",
        f"Production snapshot SHA256: `{prepared['production_header_sha256']}`. Source/runner/binary/input hashes and commands are in `prepared.json`; compiler results and gates in `compile.json`/`small-checks.json`.", "",
        f"Timing window: {metadata['started_at']} – {metadata['finished_at']}. {metadata['kernel_profiles']} kernel and {metadata['io_profiles']} I/O profiles, including {metadata['warmup_profiles']} retained warmups. Five measured samples per cell. No cross-compiler/mode pooling.", "",
        "The CPU is shared and unpinned. Median changes below are observations; separated sample ranges are descriptive, not statistical confidence intervals. Negative percentages mean less elapsed time. Full calls include input-copy/allocation/cleanup; stage proxies come from a separate independent caliper decomposition and are not additive production-call timings.", "",
        "## Full-call comparison range by compiler/mode", "",
        "| Compiler | Mode | Candidate | Median delta range | Faster cells | Separated faster/slower |",
        "|---|---|---|---:|---:|---:|"]
    for compiler in sorted({row['compiler'] for row in comparisons}):
        for mode in ["release", "assert"]:
            for variant in ["indices", "records", "fast-cases"]:
                rows = [r for r in comparisons if r['compiler'] == compiler and r['mode'] == mode and r['variant'] == variant and r['stage'] == 'full_ns']
                lines.append(f"| {compiler} | {mode} | {variant} | {min(r['delta_percent'] for r in rows):+.2f}% … {max(r['delta_percent'] for r in rows):+.2f}% | {sum(r['delta_percent'] < 0 for r in rows)}/{len(rows)} | {sum(r['ranges_separated_faster'] for r in rows)}/{sum(r['ranges_separated_slower'] for r in rows)} |")
    lines += ["", "## Fast-case shortcut: all cells", "",
        "| Compiler | Mode | Input | N parameter | Before ms [min,max] | Wrapper ms [min,max] | Delta |",
        "|---|---|---|---:|---:|---:|---:|"]
    def cell(r):
        return f"| {r['compiler']} | {r['mode']} | {r['workload']} | {r['size']} | {r['baseline_median_ns']/1e6:.6f} [{r['baseline_min_ns']/1e6:.6f},{r['baseline_max_ns']/1e6:.6f}] | {r['candidate_median_ns']/1e6:.6f} [{r['candidate_min_ns']/1e6:.6f},{r['candidate_max_ns']/1e6:.6f}] | {r['delta_percent']:+.2f}% |"
    lines += [cell(r) for r in comparisons if r['stage'] == 'full_ns' and r['variant'] == 'fast-cases']
    lines += ["", "## Slower separated full-call comparisons", "",
        "All full-call sample-separated regressions are retained here. Other overlapping regressions remain in the complete CSV.", "",
        "| Compiler | Mode | Input | N parameter | Candidate | Before ms | Candidate ms | Delta |",
        "|---|---|---|---:|---|---:|---:|---:|"]
    for r in comparisons:
        if r['stage'] == 'full_ns' and r['ranges_separated_slower']:
            lines.append(f"| {r['compiler']} | {r['mode']} | {r['workload']} | {r['size']} | {r['variant']} | {r['baseline_median_ns']/1e6:.6f} | {r['candidate_median_ns']/1e6:.6f} | {r['delta_percent']:+.2f}% |")
    lines += ["", "## Stage, I/O and memory interpretation", "",
        "`stage-comparisons.csv` includes every comparable construction/calipers/recovery and I/O phase median/min/max. Index/record layouts have no original-index recovery pass; null phases are not replaced with zero. Tiny/fast-cases have no staged decomposition. Kernel stage timing excludes input generation and objective checks; external I/O end-to-end includes process startup/pipes and objective validation after formatting. All input cases are retained to separate phases, unlike the streaming official verifier.", "",
        "Allocation diagnostics are separate GCC binaries and do not contribute timings. Calls/bytes describe full algorithm calls only; bytes are cumulative requested payload, not peak-live bytes. RSS is Linux whole-process high water beneath a small parent, includes input/runtime/allocator retention and cannot establish precise algorithm-only savings. Diagnostic objectives are checked against timing identities.", "",
        "The successful small gate contains all240 cross-variant profiles and six1,258-case brute runs. All12 invalid-singleton statuses were independently verified as SIGABRT(-6), although the runner accepts any nonzero status. A hypothetical small-gate JSON/schema failure would retain the rejected command/output but not earlier in-memory small rows; no such failure occurred in this experiment.", "",
        "Cached KACTL/maspypy/official source contracts and licenses are described in `../research-notes.md`. The original Fastest403 remains a documented limitation; no leaderboard speed comparison or network retry was performed in this resumed performance experiment.", ""]
    (folder / "comparison-report.md").write_text("\n".join(lines))
    print(f"Wrote {len(comparisons)} LF CSV stage comparisons and report.")


if __name__ == "__main__":
    main()
