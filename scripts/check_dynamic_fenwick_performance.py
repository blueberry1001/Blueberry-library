#!/usr/bin/env python3
"""Gate the isolated Dynamic Fenwick benchmark against pinned Nyaan sources."""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import statistics
import sys


SHAPES = ("contest-near", "contest-diffuse", "contest-broad")
VARIANTS = ("candidate", "nyaan")
EXPECTED_REFERENCE = {
    "nyaan": "e682c3e115bb149d626b6aaf59e317907cfe66d97a61e6a0c20e87740c5678f8",
    "nyaan_hash": "a60e79ed9e8950fe23338c749aaf924883837e6ae9188d4e7d46185c08a14a8b",
}
THRESHOLD = 2.0


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def assess(result, candidate, driver):
    if not isinstance(result, dict):
        raise ValueError("invalid benchmark report")
    if (result.get("count") != 300000 or result.get("runs") != 3 or result.get("seed") != 1729 or
            result.get("flags") != ["-std=gnu++20", "-O2", "-DNDEBUG"]):
        raise ValueError("unexpected benchmark inputs, repetitions, or compiler flags")
    for key in ("compiler", "platform", "cpu"):
        if not isinstance(result.get(key), str) or not result[key]:
            raise ValueError(f"missing environment field: {key}")
    expected = {**EXPECTED_REFERENCE, "candidate": digest(candidate), "driver": digest(driver)}
    if result.get("sha256") != expected:
        raise ValueError("source hashes differ from the pinned Nyaan or current candidate/driver")
    samples = result.get("measurements")
    if not isinstance(samples, list) or len(samples) != len(SHAPES) * len(VARIANTS) * 3:
        raise ValueError("incomplete benchmark measurements")
    seen, checksums, durations = set(), {}, {}
    for sample in samples:
        if not isinstance(sample, dict):
            raise ValueError("invalid benchmark sample")
        shape, variant, run = sample.get("shape"), sample.get("variant"), sample.get("run")
        key = (shape, variant, run)
        if shape not in SHAPES or variant not in VARIANTS or type(run) is not int or run not in (1, 2, 3) or key in seen:
            raise ValueError("unexpected or duplicate benchmark sample")
        seen.add(key)
        duration, rss, checksum = sample.get("milliseconds"), sample.get("peak_rss_kib"), sample.get("checksum")
        if (isinstance(duration, bool) or not isinstance(duration, (int, float)) or
                not math.isfinite(duration) or duration <= 0 or type(rss) is not int or rss <= 0 or
                type(checksum) is not int):
            raise ValueError("invalid benchmark sample")
        if shape in checksums and checksums[shape] != checksum:
            raise ValueError(f"checksum mismatch in {shape}")
        checksums[shape] = checksum
        durations.setdefault((shape, variant), []).append(duration)
    rows = []
    for shape in SHAPES:
        candidate_ms = statistics.median(durations[shape, "candidate"])
        nyaan_ms = statistics.median(durations[shape, "nyaan"])
        rows.append({"shape": shape, "candidate_ms": candidate_ms, "nyaan_ms": nyaan_ms,
                     "ratio": candidate_ms / nyaan_ms, "checksum": checksums[shape]})
    return {"status": "failed" if any(row["ratio"] > THRESHOLD for row in rows) else "passed",
            "threshold": THRESHOLD, "rows": rows,
            "reference_revision": "b3981adc80a800b2584980b01821324ea6c77183"}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results", type=Path, required=True)
    parser.add_argument("--candidate", type=Path, required=True)
    parser.add_argument("--driver", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        result = json.loads(args.results.read_text(encoding="utf-8"))
        assessment = assess(result, args.candidate, args.driver)
    except (OSError, ValueError, KeyError, TypeError) as error:
        assessment = {"status": "not_measured", "reason": str(error), "threshold": THRESHOLD}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(assessment, indent=2) + "\n", encoding="utf-8")
    summary = ["## Dynamic Fenwick performance regression check", "",
               "Candidate and fixed Nyaan sources run on the same runner, compiler and inputs. "
               "The threshold is 2× the reference median across three timed runs after one warmup. "
               "This covers only the three documented synthetic integral workloads.", ""]
    for row in assessment.get("rows", []):
        summary.append(f"- {row['shape']}: {row['candidate_ms']:.3f} ms / {row['nyaan_ms']:.3f} ms = {row['ratio']:.3f}×")
    summary += ["", f"Status: **{assessment['status']}**", ""]
    if assessment.get("reason"):
        summary += [f"Reason: {assessment['reason']}", ""]
    message = "\n".join(summary)
    print(message)
    if os.environ.get("GITHUB_STEP_SUMMARY"):
        with Path(os.environ["GITHUB_STEP_SUMMARY"]).open("a", encoding="utf-8") as stream:
            stream.write(message + "\n")
    if assessment["status"] != "passed":
        print("::error::Dynamic Fenwick performance comparison failed or was not measured")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
