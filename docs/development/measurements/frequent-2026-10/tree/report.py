#!/usr/bin/env python3
"""Regenerate selected before/after tables from immutable raw-run summaries."""
import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
rows = json.loads((HERE / "full/summary.json").read_text())
lookup = {(r["profile"], r["variant"], r["kind"], r["shape"]): r for r in rows}
lines = [
    "# LCA measurement results", "",
    "Baseline: `d6af3e9`. N=200000, Q=500000, seed=20261017, 1 warmup + 5 recorded runs. "
    "These are medians from `full/summary.json`; raw attempts including warmups are in `full/samples.jsonl`. "
    "Build and query durations exclude tree/query generation and I/O. Peak RSS includes shared input arrays.", "",
    "Final selection: `bit-scan.hpp` for Doubling/kth and the identical RMQ portion of `candidate.hpp`. "
    "`final.hpp` combines these changes. All checksum comparisons passed.", "",
]
for profile in ["gcc-release", "gcc-assert", "clang-release"]:
    lines += ["## " + profile, "",
              "| API | Shape | Build ms before -> after | Query ms before -> after | Total ms before -> after | Peak RSS KiB before -> after |",
              "| --- | --- | ---: | ---: | ---: | ---: |"]
    for kind in ["doubling", "rmq", "kth"]:
        for shape in ["path", "star", "balanced", "broom", "random", "permuted"]:
            before = lookup[profile, "baseline", kind, shape]
            after = lookup[profile, "candidate" if kind == "rmq" else "bit-scan", kind, shape]
            cells = [kind, shape]
            for metric in ["build_ms", "query_ms", "total_ms"]:
                cells.append("%.3f -> %.3f" % (before[metric]["median"], after[metric]["median"]))
            cells.append("%d -> %d" % (before["peak_rss_kib"]["median"], after["peak_rss_kib"]["median"]))
            lines.append("| " + " | ".join(cells) + " |")
    lines.append("")
lines += [
    "## Selection and limits", "",
    "The packed 64-bit parent rank/vertex candidate is preserved in `packed.hpp`. "
    "`packed-retry/` contains a separate 5-run same-input comparison against baseline and int. "
    "Packing can remove the dependent vertex lookup and improve some query loops, but increases peak RSS "
    "by about 13 MiB relative to int and generally increases construction plus query time; it was rejected. "
    "The first attempt in `packed/run.log` exited 127 because temporary baseline binaries were no longer present. "
    "No failed/missing result receives a timing. Binaries were rebuilt under `.benchmark/frequent-tree-binaries`.", "",
    "The int RMQ path query loop regressed in the original full run: GCC release 4.94 -> 5.46 ms and "
    "Clang release 4.82 -> 5.32 ms. The additional run did not reproduce the GCC regression "
    "(5.17 -> 4.89 ms), while Clang retained it (4.97 -> 5.34 ms). Construction and memory savings "
    "dominate at this Q/N ratio. Very query-heavy path inputs can prefer the old query layout; "
    "no universal speedup is claimed.", "",
    "`small/` independently measures N=4096, Q=100000 with GCC/Clang release, the same 6 shapes, "
    "and 5 repeats after warmup. Every median construction + query time improved. "
    "GCC total reductions: Doubling 20.4-82.3%, RMQ 27.8-32.5%, kth 13.9-70.6%. "
    "Clang total reductions: Doubling 48.4-87.9%, RMQ 28.8-62.7%, kth 17.5-70.0%. "
    "Sub-millisecond measurements are noisy; inspect raw samples before interpreting minor differences.", "",
    "Focused correctness: `validation.json`/`validation.log` contain 106 successful compile/run commands. "
    "GCC/Clang x gnu++20/23 each compile and execute the new boundary/random test with seeds 1-20 plus "
    "existing `tests/tree_random.cpp`, compile both official LCA verify drivers, and compile a duplicate "
    "standalone include with `-DNDEBUG`. GCC AddressSanitizer+UndefinedBehaviorSanitizer passed seed 20261017. "
    "Repository-wide checks and official testcase execution are performed by the integration owner.", "",
    "The original full-run script is preserved as `full/run-script.py`; its SHA256 matches the run.py hash "
    "recorded in that environment. Current `run.py` adds kind/shape filters for later comparisons. "
    "Snapshot headers keep SparseTable at the baseline revision, so the LCA effects are isolated.", "",
]
(HERE / "REPORT.md").write_text("\n".join(lines), encoding="utf-8")
