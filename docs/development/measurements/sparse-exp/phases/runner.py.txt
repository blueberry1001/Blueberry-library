#!/usr/bin/env python3
"""Compare unmodified Fastest submissions and Blueberry on identical official files."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import signal
import statistics
import subprocess
import threading
import time


def sha256(path):
    h = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def validate(actual, expected):
    # Numeric comparison accepts leading zeroes and different legal whitespace.
    wanted = expected.read_bytes().split()
    got = actual.read_bytes().split()
    if len(wanted) != len(got):
        return f"coefficient count: expected {len(wanted)}, actual {len(got)}"
    for i, (a, b) in enumerate(zip(wanted, got)):
        try:
            if int(a) != int(b):
                return f"coefficient {i}: expected {a[:40]!r}, actual {b[:40]!r}"
        except ValueError:
            return f"non-integer coefficient at {i}"
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cases", required=True, type=Path)
    parser.add_argument("--source-dir", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path, help="new output directory")
    parser.add_argument("--acl", type=Path, help="directory containing atcoder/; default .deps/ac-library")
    parser.add_argument("--compilers", nargs="+", default=["g++", "clang++"])
    parser.add_argument("--submissions", nargs="+", default=["409367"])
    parser.add_argument("--case", action="append", dest="patterns", help="glob relative to --cases; first match selected per pattern")
    parser.add_argument("--all-cases", action="store_true", help="run every *.in in the official test directory")
    parser.add_argument("--runs", type=int, default=3)
    parser.add_argument("--warmups", type=int, default=1)
    parser.add_argument("--timeout", type=float, default=120)
    parser.add_argument("--phases", action="store_true", help="instrument Blueberry's actual parse/algorithm/format+flush phases")
    args = parser.parse_args()
    if args.runs < 1 or args.warmups < 0:
        parser.error("runs must be positive; warmups must be nonnegative")
    if any(not re.fullmatch(r"\d+", x) for x in args.submissions):
        parser.error("submission IDs must be decimal integers")
    root = Path(__file__).resolve().parents[2]
    acl = (args.acl or root / ".deps/ac-library").resolve()
    args.output.mkdir(parents=True, exist_ok=False)
    flags = ["-std=gnu++20", "-O2", "-DNDEBUG", "-mavx2", "-mbmi", "-mbmi2"]
    meta = {"platform": platform.platform(), "python": platform.python_version(), "flags": flags,
            "cpu_affinity": sorted(os.sched_getaffinity(0)),
            "runs": args.runs, "warmups": args.warmups, "compilers": {}, "cases": [], "sources": {}, "errors": [],
            "timing_scope": "end-to-end process with official input/output files, including startup, parsing, algorithm, formatting and file I/O",
            "rss_method": "/usr/bin/time -f %M, KiB", "source_changes_allowed": False,
            "source_restriction": "Fastest originals compiled unmodified; ISA flags applied identically to both implementations"}
    if Path("/proc/cpuinfo").exists():
        meta["cpu"] = next((s.split(":", 1)[1].strip() for s in Path("/proc/cpuinfo").read_text().splitlines()
                            if s.startswith("model name")), "unavailable")
    meta["git_head"] = subprocess.run(["git", "rev-parse", "HEAD"], cwd=root, capture_output=True, text=True).stdout.strip()
    samples = []
    metadata_path = args.output / "metadata.json"
    write_json(metadata_path, meta)
    try:
        if not Path("/usr/bin/time").exists():
            raise RuntimeError("/usr/bin/time is required")
        selected = []
        if args.all_cases:
            selected = sorted(args.cases.glob("*.in"))
        else:
            for pattern in args.patterns or ["max_random_*.in", "min_K_*.in", "small_dense_*.in"]:
                matches = sorted(p for p in args.cases.glob(pattern) if p.is_file() and p.suffix == ".in")
                if not matches:
                    raise RuntimeError(f"no official input matches {pattern!r} in {args.cases}")
                selected.append(matches[0])
        selected = sorted(set(selected))
        if len(selected) < 3:
            raise RuntimeError("at least three distinct official inputs are required")
        for path in selected:
            expected = path.with_suffix(".out")
            if not expected.is_file():
                raise RuntimeError(f"missing official expected output: {expected}")
            meta["cases"].append({"name": path.name, "input": str(path.resolve()), "expected": str(expected.resolve()),
                                  "input_sha256": sha256(path), "expected_sha256": sha256(expected)})
        sources = {"blueberry": root / ("benchmarks/sparse-exp/phase_driver.cpp" if args.phases else "verify/math/sparse-exp.test.cpp")}
        sources.update({f"submission-{x}": args.source_dir / f"submission-{x}.cpp" for x in args.submissions})
        dependencies = [root / "blueberry/math/sparse-exp.hpp", root / "blueberry/math/formal-power-series.hpp",
                        root / "blueberry/utility/fast-io.hpp", Path(__file__).resolve()]
        dependencies.extend(acl / "atcoder" / x for x in ("modint.hpp", "convolution.hpp", "internal_math.hpp"))
        for source in list(sources.values()) + dependencies:
            if not source.is_file():
                raise RuntimeError(f"missing source/dependency: {source}")
            meta["sources"][str(source.resolve())] = sha256(source)
        write_json(metadata_path, meta)
        with (args.output / "samples.jsonl").open("x", encoding="utf-8") as raw:
            for index, compiler in enumerate(args.compilers):
                info = meta["compilers"].setdefault(compiler, {})
                executable = shutil.which(compiler)
                if executable is None:
                    info["error"] = "compiler not found"
                    meta["errors"].append(f"{compiler}: not found")
                    write_json(metadata_path, meta)
                    continue
                info["version"] = subprocess.run([executable, "--version"], capture_output=True, text=True, check=True).stdout
                binaries = {}
                info["builds"] = {}
                for label, source in sources.items():
                    prefix = args.output / f"compiler-{index}-{label}"
                    binary = prefix.with_suffix(".exe")
                    command = [executable, *flags, "-I", str(root), "-I", str(acl), str(source.resolve()), "-o", str(binary.resolve())]
                    build = {"command": command}
                    info["builds"][label] = build
                    try:
                        with prefix.with_suffix(".compile.stdout").open("wb") as stdout, prefix.with_suffix(".compile.stderr").open("wb") as stderr:
                            proc = subprocess.run(command, stdout=stdout, stderr=stderr, timeout=240)
                        build["returncode"] = proc.returncode
                        if proc.returncode:
                            meta["errors"].append(f"{compiler}/{label}: compilation failed")
                        else:
                            build["binary_sha256"] = sha256(binary)
                            binaries[label] = binary
                    except (subprocess.TimeoutExpired, OSError) as exc:
                        build["error"] = str(exc)
                        meta["errors"].append(f"{compiler}/{label}: {exc}")
                    write_json(metadata_path, meta)
                # Timing only a surviving implementation would produce a misleading comparison.
                if len(binaries) != len(sources):
                    continue
                for path in selected:
                    for run in range(-args.warmups, args.runs):
                        for label, binary in binaries.items():
                            stem = f"compiler-{index}-{label}-{path.stem}-run{run}"
                            output = args.output / (stem + ".stdout")
                            stderr = args.output / (stem + ".stderr")
                            rss = args.output / (stem + ".rss")
                            row = {"compiler": compiler, "implementation": label, "case": path.name,
                                   "run": run, "warmup": run < 0, "output_file": str(output), "stderr_file": str(stderr)}
                            command = ["/usr/bin/time", "-f", "%M", "-o", str(rss.resolve()), str(binary.resolve())]
                            row["command"] = command
                            start = time.perf_counter()
                            try:
                                with path.open("rb") as stdin, output.open("wb") as stdout, stderr.open("wb") as err:
                                    proc = subprocess.Popen(command, stdin=stdin, stdout=stdout, stderr=err, start_new_session=True)
                                    timed_out = threading.Event()
                                    def stop():
                                        timed_out.set()
                                        try:
                                            os.killpg(proc.pid, signal.SIGKILL)
                                        except ProcessLookupError:
                                            pass
                                    watchdog = threading.Timer(args.timeout, stop)
                                    watchdog.daemon = True
                                    watchdog.start()
                                    try:
                                        # Blocking wait avoids timeout polling's millisecond quantization.
                                        proc.wait()
                                    finally:
                                        watchdog.cancel()
                                    if timed_out.is_set():
                                        raise subprocess.TimeoutExpired(command, args.timeout)
                                row.update(wall_ms=(time.perf_counter() - start) * 1000, returncode=proc.returncode)
                                if proc.returncode:
                                    row["error"] = "execution failed"
                                else:
                                    row["peak_rss_kib"] = int(rss.read_text().strip())
                                    row["output_sha256"] = sha256(output)
                                    if args.phases and label == "blueberry":
                                        row.update(json.loads(stderr.read_text()))
                                        if not row["flush_ok"]:
                                            row["error"] = "output flush failed"
                                    error = validate(output, path.with_suffix(".out"))
                                    if error:
                                        row["error"] = error
                            except (subprocess.TimeoutExpired, OSError, ValueError) as exc:
                                row["error"] = str(exc)
                            if "error" in row:
                                meta["errors"].append(f"{stem}: {row['error']}")
                            else:
                                # Keep hashes and all stderr/RSS logs; full failed output is retained.
                                output.unlink()
                                row["successful_output_removed"] = True
                            samples.append(row)
                            raw.write(json.dumps(row, sort_keys=True) + "\n")
                            raw.flush()
                    print(f"{compiler}: {path.name}", flush=True)
                write_json(metadata_path, meta)
        changed = [p for p, h in meta["sources"].items() if sha256(Path(p)) != h]
        meta["sources_unchanged_during_run"] = not changed
        if changed:
            meta["errors"].append(f"sources changed during run: {changed}")
    except (RuntimeError, OSError, subprocess.SubprocessError) as exc:
        meta["errors"].append(str(exc))
    finally:
        groups = {}
        for row in samples:
            if not row["warmup"] and "error" not in row:
                groups.setdefault((row["compiler"], row["implementation"], row["case"]), []).append(row)
        summary = []
        for (compiler, implementation, case), rows in groups.items():
            item = {"compiler": compiler, "implementation": implementation, "case": case, "samples": len(rows)}
            for metric in ("wall_ms", "peak_rss_kib", "parse_ms", "algorithm_ms", "format_ms", "total_ms"):
                values = [r[metric] for r in rows if metric in r]
                if values:
                    item[metric] = {"median": statistics.median(values), "min": min(values)}
            summary.append(item)
        write_json(args.output / "summary.json", summary)
        write_json(metadata_path, meta)
    if meta["errors"]:
        raise SystemExit(f"comparison incomplete: {len(meta['errors'])} errors; inspect metadata.json and retained logs")


if __name__ == "__main__":
    main()
