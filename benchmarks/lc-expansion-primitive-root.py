#!/usr/bin/env python3
"""Comparable primitive-root candidates, official input, split timing and RSS."""
import hashlib
import json
from pathlib import Path
import platform
import statistics
import subprocess
import time
from urllib.request import urlopen

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / ".build/primitive-root"
OUT = ROOT / "benchmarks/research/lc-expansion-primitive-root"
CASES = ROOT / ".verify-helper/cache/4b2bc2fc2e6cf41ff88c771fa6a4180b/test"
FAST = BUILD / "references/submission-410843.cpp"


def command(args, **kwargs):
    result = subprocess.run(args, cwd=ROOT, capture_output=True, text=True, **kwargs)
    if result.returncode:
        raise RuntimeError(f"{args}: {result.stderr}")
    return result


def summary(samples):
    return {"median": statistics.median(samples), "min": min(samples), "samples": samples}


def main():
    BUILD.mkdir(parents=True, exist_ok=True)
    if not FAST.exists():
        # Keep the public comparison source outside the CC0 source tree.
        with urlopen("https://v3.api.judge.yosupo.jp/submissions/410843", timeout=30) as response:
            source = json.load(response)["source"]
        FAST.parent.mkdir(parents=True, exist_ok=True)
        FAST.write_text(source)
    expected = "85c7617368b71169636bc57fea27b6afb77678d1fdb2ad19d691681245ffd743"
    if hashlib.sha256(FAST.read_bytes()).hexdigest() != expected:
        raise RuntimeError("Fastest comparison source does not match submission 410843")
    cases = sorted(CASES.glob("*.in"))
    primes = []
    for case in cases:
        values = list(map(int, case.read_text().split()))
        assert values[0] == len(values) - 1
        primes.extend(values[1:])
    data = str(len(primes)) + "\n" + "\n".join(map(str, primes)) + "\n"
    repeated = primes * 20
    full_input = str(len(repeated)) + "\n" + "\n".join(map(str, repeated)) + "\n"
    report = {
        "cpu": next(line.split(":", 1)[1].strip() for line in Path("/proc/cpuinfo").read_text().splitlines() if line.startswith("model name")),
        "platform": platform.platform(), "runs": 5, "operation_repeats": 10,
        "queries": len(primes), "end_to_end_queries": len(repeated),
        "input_sha256": hashlib.sha256(data.encode()).hexdigest(),
        "sources": {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in [ROOT / "blueberry/math/primitive-root.hpp", ROOT / "blueberry/math/factorize.hpp", FAST]},
        "results": [],
    }
    for compiler in ["g++", "clang++"]:
        for mode in ["assert", "release"]:
            # The external submission uses C++23 multidimensional operator[].
            flags = ["-std=gnu++23", "-O2", "-I."] + (["-DNDEBUG"] if mode == "release" else [])
            compiled = {}
            compile_times = {}
            for name, source in [("candidates", "benchmarks/lc-expansion-primitive-root.cpp"), ("blueberry", "verify/math/primitive-root.test.cpp"), ("fastest", str(FAST))]:
                target = BUILD / f"{compiler}-{mode}-{name}"
                start = time.perf_counter()
                command([compiler, *flags, source, "-o", str(target)])
                compile_times[name] = time.perf_counter() - start
                compiled[name] = target
            record = {"compiler": command([compiler, "--version"]).stdout.splitlines()[0], "flags": flags, "mode": mode, "compile_seconds": compile_times, "split_ms": {}, "end_to_end_ms": {}, "peak_rss_kib": {}}
            samples = {}
            raw = []
            # Warm all candidates once before recording.
            command([str(compiled["candidates"]), "1"], input=data)
            for _ in range(5):
                result = command([str(compiled["candidates"]), "10"], input=data)
                raw.append(result.stdout)
                for line in result.stdout.splitlines():
                    fields = line.split()
                    if len(fields) == 2:
                        samples.setdefault(fields[0], []).append(float(fields[1]))
            record["split_ms"] = {name: summary(values) for name, values in samples.items()}
            for name in ["blueberry", "fastest"]:
                timings, memory = [], []
                for run in range(5):
                    rss = BUILD / f"rss-{compiler}-{mode}-{name}-{run}.txt"
                    start = time.perf_counter()
                    result = command(["/usr/bin/time", "-f", "%M", "-o", str(rss), str(compiled[name])], input=full_input)
                    timings.append((time.perf_counter() - start) * 1000)
                    assert len(result.stdout.splitlines()) == len(repeated)
                    memory.append(int(rss.read_text()))
                record["end_to_end_ms"][name] = summary(timings)
                record["peak_rss_kib"][name] = summary(memory)
            (OUT / f"raw-{compiler}-{mode}.txt").write_text("\n".join(raw))
            report["results"].append(record)
            print(json.dumps(record), flush=True)
    (OUT / "benchmark.json").write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    main()
