#!/usr/bin/env python3
"""Focused same-binary confirmation; preserves the original 864 profiles."""
import importlib.util
import json
from pathlib import Path
import statistics
from datetime import datetime, timezone

SOURCE = Path(__file__).with_name("representative-performance.py")
spec = importlib.util.spec_from_file_location("survey", SOURCE)
survey = importlib.util.module_from_spec(spec)
spec.loader.exec_module(survey)
OUT = survey.RESULTS / "focused-recheck"


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    if (OUT / "prepared.json").exists():
        raise RuntimeError("Preserve existing focused recheck before reproducing")
    groups = [("dense", mode, ("segment-tree", "random", "u64", 100000, 200000), None)
              for mode in ["release", "assert"]]
    groups += [("persistent-segment", "release", ("persistent-segment", "dense-sequential", scalar, 100000, 200000), "g++")
               for scalar in ["u64", "affine-u64"]]
    groups += [("dynamic-fenwick", "assert", ("dynamic-fenwick", "boundary", "u64", 30000, 200000), "clang++")]
    ready, metadata = [], []
    for family, mode, case, compiler in groups:
        data = survey.prepared(family, mode)
        survey.checked_correctness(family, mode)
        entries = [(entry, implementation) for entry, implementation in survey.entries_for(data, family)
                   if compiler is None or entry["compiler"] == compiler]
        for entry, _ in entries: entry["failure_directory"] = str(OUT)
        _, original = survey.location(family, mode)
        metadata.append(dict(family=family, mode=mode, case=case, compiler_filter=compiler,
                             original_prepared_sha256=survey.digest(original / "prepared.json"),
                             entries=entries))
        ready.append((family, mode, case, entries))
    survey.write(OUT, "prepared.json", dict(created_at=datetime.now(timezone.utc).isoformat(),
        baseline_commit=survey.BASELINE, repeats=9, warmups=1, groups=metadata,
        runner_sha256=survey.digest(Path(__file__)), shared_runner_sha256=survey.digest(SOURCE),
        conditions="Same frozen binaries/input cases; no rebuild. Original source/header/binary hashes and small gates verified. "
        "Nine alternating samples after one warmup; original experiment unchanged."))
    survey.quiet(OUT)
    rows, summary = [], []
    started = datetime.now(timezone.utc)
    for family, mode, case, entries in ready:
        expected = None
        for repeat in range(-1, 9):
            for entry, implementation in entries if repeat % 2 else entries[::-1]:
                row = survey.execute(entry, case, implementation, False)
                if expected is None: expected = survey.identity(row)
                if survey.identity(row) != expected:
                    survey.write(OUT, "failed-profile.json", dict(rejected=row, expected=expected))
                    raise RuntimeError("Focused checksum disagreement")
                row.update(mode=mode, repeat=repeat, warmup=repeat < 0)
                rows.append(row); survey.write(OUT, "raw.json", rows)
        for entry, implementation in entries:
            selected = [r for r in rows if not r["warmup"] and
                        (r["family"], r["shape"], r["scalar"], r["mode"], r["compiler"], r["implementation"]) ==
                        (*case[:3], mode, entry["compiler"], implementation)]
            fields = [k for k in selected[0] if k.endswith("_ns")]
            metrics = {k: dict(median=statistics.median(r[k] for r in selected),
                               min=min(r[k] for r in selected), max=max(r[k] for r in selected)) for k in fields}
            summary.append(dict(family=case[0], shape=case[1], scalar=case[2], mode=mode,
                                compiler=entry["compiler"], implementation=implementation, metrics=metrics))
        print(f"Completed focused {family}/{mode}/{case[2]}", flush=True)
    survey.write(OUT, "results.json", dict(started_at=started.isoformat(), finished_at=datetime.now(timezone.utc).isoformat(),
                                         repeats=9, warmups=1, summary=summary))
    print(f"Completed {len(rows)} focused profiles including warmups; all checksums agree.", flush=True)


if __name__ == "__main__": main()
