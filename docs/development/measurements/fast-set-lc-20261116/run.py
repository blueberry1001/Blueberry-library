#!/usr/bin/env python3
"""Compare a pinned public submission and the local predecessor verify program."""
import argparse
import filecmp
import hashlib
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess
import tempfile
import time
import urllib.request

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
SUBMISSIONS = {
    "403023": "59fd15abf763b5a59bb2069e1916e21ccfebbe291d116fb5240690cbce6357d7",
    "278227": "ef5b067f04c7686cce6a75ccb1426686d0a152b4afe6599f3ddeaeb39a90219a",
}
SHAPES = ("empty", "full", "half", "sparse", "cluster", "updates")


def digest(path):
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--output", type=Path, required=True)
    p.add_argument("--submission", choices=SUBMISSIONS, default="403023")
    p.add_argument("--external", type=Path, help="previously fetched source, with the pinned SHA-256")
    p.add_argument("--compiler", default="g++")
    p.add_argument("--n", type=int, default=1000000)
    p.add_argument("--q", type=int, default=500000)
    p.add_argument("--seed", type=int, default=1729)
    p.add_argument("--runs", type=int, default=3)
    p.add_argument("--timeout", type=float, default=120)
    args = p.parse_args()
    if not (1 <= args.n <= 10000000 and 1 <= args.q <= 1000000 and args.runs >= 1 and args.timeout > 0):
        p.error("n/q must be within official limits; runs and timeout must be positive")
    if args.output.exists():
        p.error("output already exists; choose a new path to preserve existing measurements")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    flags = ["-std=gnu++23", "-O2", "-DNDEBUG", "-mavx2", "-mpopcnt"]
    driver = ROOT / "verify/data-structure/fast-set.test.cpp"
    headers = [ROOT / "blueberry/data-structure/fast-set.hpp", ROOT / "blueberry/utility/fast-io.hpp"]
    data = {"status": "started", "submission": args.submission,
            "submission_url": f"https://judge.yosupo.jp/submission/{args.submission}",
            "timestamp_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
            "platform": platform.platform(), "cpu": platform.processor(), "flags": flags,
            "n": args.n, "q": args.q, "seed": args.seed, "runs": args.runs,
            "hashes": {str(x.relative_to(ROOT)): digest(x) for x in [Path(__file__), HERE / "generate.cpp", HERE / "phases.cpp", driver, *headers]},
            "compile": [], "cases": [], "samples": []}

    def save():
        args.output.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")

    save()
    try:
        data["affinity"] = {"status": "unavailable"}
        try:
            eligible = sorted(os.sched_getaffinity(0))
            data["affinity"]["eligible_before"] = eligible
            if not eligible:
                raise RuntimeError("no eligible CPUs")
            selected = eligible[0]
            data["affinity"]["selected_cpu"] = selected
            os.sched_setaffinity(0, {selected})
            actual = sorted(os.sched_getaffinity(0))
            data["affinity"]["actual"] = actual
            if actual != [selected]:
                raise RuntimeError(f"requested CPU {selected}, got {actual}")
            data["affinity"]["status"] = "set"
        except (AttributeError, OSError, RuntimeError) as exc:
            data["affinity"]["error"] = f"{type(exc).__name__}: {exc}"
        save()
        data["compiler"] = subprocess.check_output([args.compiler, "--version"], text=True).splitlines()[0]
        if "clang" in data["compiler"].lower():
            flags.append("-fconstexpr-steps=100000000")
        cpu = Path("/proc/cpuinfo")
        if cpu.exists():
            lines = cpu.read_text().splitlines()
            data["cpu"] = next((line.split(":", 1)[1].strip() for line in lines if line.startswith("model name")), data["cpu"])
            features = set(next((line.split(":", 1)[1].split() for line in lines if line.startswith("flags")), []))
            data["cpu_features"] = {feature: feature in features for feature in ("avx2", "popcnt")}
            if not {"avx2", "popcnt"}.issubset(features):
                raise RuntimeError("CPU does not report both AVX2 and POPCNT; measurements not run")
        else:
            raise RuntimeError("cannot check CPU features; measurements not run")
        with tempfile.TemporaryDirectory(prefix="blueberry-fast-set-") as name:
            tmp = Path(name)
            external = tmp / "external.cpp"
            if args.external:
                raw = args.external.read_bytes()
                data["hashes"]["external_supplied_bytes"] = hashlib.sha256(raw).hexdigest()
                external.write_bytes(raw.replace(b"\r\n", b"\n"))
            else:
                req = urllib.request.Request(f"https://v3.api.judge.yosupo.jp/submissions/{args.submission}", headers={"User-Agent": "Blueberry-library/performance-study"})
                with urllib.request.urlopen(req, timeout=30) as response:
                    source = json.load(response)["source"]
                external.write_bytes(source.encode("utf-8"))
            data["hashes"]["external"] = digest(external)
            if data["hashes"]["external"] != SUBMISSIONS[args.submission]:
                raise RuntimeError("submission source SHA-256 does not match the pinned value")
            binaries = {"candidate": driver, "external": external, "phases": HERE / "phases.cpp", "generator": HERE / "generate.cpp"}
            for label, source in binaries.items():
                command = [args.compiler, *flags, "-I", str(ROOT), str(source), "-o", str(tmp / label)]
                proc = subprocess.run(command, capture_output=True, text=True, timeout=args.timeout)
                data["compile"].append({"name": label, "command": command, "exit": proc.returncode, "stdout": proc.stdout, "stderr": proc.stderr})
                save()
                if proc.returncode:
                    raise RuntimeError(f"{label} failed to compile")
            case_specs = [(shape, args.n, args.q) for shape in SHAPES]
            case_specs += [("half", n, min(args.q, 1000)) for n in (1, 63, 64, 65, 4095, 4096, 4097)]
            for case_id, (shape, n, q) in enumerate(case_specs):
                inp, expected, actual, metrics = (tmp / x for x in ("case.in", "expected.out", "actual.out", "time.txt"))
                command = [str(tmp / "generator"), str(n), str(q), shape, str(args.seed), str(inp), str(expected)]
                start = time.perf_counter()
                proc = subprocess.run(command, capture_output=True, text=True, timeout=args.timeout)
                generation_ms = (time.perf_counter() - start) * 1000
                case = {"id": case_id, "shape": shape, "n": n, "q": q, "generation_and_oracle_ms": generation_ms, "generator_exit": proc.returncode,
                        "generator_stderr": proc.stderr}
                data["cases"].append(case)
                if proc.returncode:
                    raise RuntimeError(f"{shape} generation failed")
                case.update(input_sha256=digest(inp), expected_sha256=digest(expected), input_bytes=inp.stat().st_size,
                            expected_bytes=expected.stat().st_size)
                save()
                for run in range(-1, args.runs):
                    variants = ["candidate", "external", "phases"]
                    offset = (run + 1) % len(variants)
                    for label in variants[offset:] + variants[:offset]:
                        metrics.unlink(missing_ok=True)
                        command = ["/usr/bin/time", "-f", "%M", "-o", str(metrics), str(tmp / label)]
                        with inp.open("rb") as source, actual.open("wb") as target:
                            start = time.perf_counter()
                            proc = subprocess.run(command, stdin=source, stdout=target, stderr=subprocess.PIPE, timeout=args.timeout)
                            wall_ms = (time.perf_counter() - start) * 1000
                        filecmp.clear_cache()
                        sample = {"case_id": case_id, "shape": shape, "n": n, "q": q, "run": run + 1, "warmup": run == -1, "variant": label,
                                  "wall_ms": wall_ms, "exit": proc.returncode, "stderr": proc.stderr.decode("utf-8", "replace"),
                                  "actual_sha256": digest(actual), "output_matches": filecmp.cmp(actual, expected, shallow=False),
                                  "rss_kib": None}
                        try:
                            rss = int(metrics.read_text().strip().splitlines()[-1])
                            if rss <= 0:
                                raise ValueError(f"RSS is {rss}, not a positive measurement")
                            sample["rss_kib"] = rss
                        except (OSError, IndexError, ValueError) as exc:
                            sample["measurement_error"] = f"RSS unavailable: {type(exc).__name__}: {exc}"
                        data["samples"].append(sample)
                        save()
                        if sample["rss_kib"] is None:
                            raise RuntimeError(f"{shape}/{label}: RSS measurement unavailable")
                        if proc.returncode or not sample["output_matches"] or sample["actual_sha256"] != case["expected_sha256"]:
                            raise RuntimeError(f"{shape}/{label} failed or disagreed with oracle")
                        if label == "phases":
                            sample["phases_ms"] = json.loads(sample["stderr"])
                        save()
            data["summary"] = []
            for case in data["cases"]:
                for label in ("candidate", "external", "phases"):
                    rows = [s for s in data["samples"] if s["case_id"] == case["id"]
                            and s["variant"] == label and not s["warmup"]]
                    summary = {"case_id": case["id"], "shape": case["shape"], "n": case["n"], "q": case["q"], "variant": label,
                               "wall_median_ms": statistics.median(s["wall_ms"] for s in rows),
                               "wall_min_ms": min(s["wall_ms"] for s in rows),
                               "rss_median_kib": statistics.median(s["rss_kib"] for s in rows)}
                    if label == "phases":
                        summary["phases_median_ms"] = {key: statistics.median(s["phases_ms"][key] for s in rows)
                                                       for key in ("parse_ms", "build_ms", "operation_ms", "output_ms")}
                    data["summary"].append(summary)
            data["status"] = "completed"
    except Exception as exc:
        data["status"] = "failed"
        data["error"] = f"{type(exc).__name__}: {exc}"
        raise
    finally:
        save()


if __name__ == "__main__":
    main()
