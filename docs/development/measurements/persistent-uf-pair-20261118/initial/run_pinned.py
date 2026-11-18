import argparse
import datetime
import hashlib
import json
import os
import platform
import random
import re
import statistics
import subprocess
import sys
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parent.parent
FILES = ["benchmark.cpp", "test_pinned.cpp", "run_pinned.py"] + [
    f"{v}/blueberry/data-structure/{name}.hpp"
    for v in ("baseline", "candidate")
    for name in ("persistent-array", "persistent-union-find")
]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def command_output(argv):
    try:
        p = subprocess.run(argv, cwd=REPO, text=True, capture_output=True)
        return {"argv": argv, "returncode": p.returncode, "stdout": p.stdout, "stderr": p.stderr}
    except OSError as e:
        return {"argv": argv, "error": str(e)}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--std", default="gnu++20")
    parser.add_argument("--n", type=int, default=131072)
    parser.add_argument("--count", type=int, default=200000)
    parser.add_argument("--seed", type=int, default=20261118)
    parser.add_argument("--runs", type=int, default=15)
    parser.add_argument("--warmups", type=int, default=2)
    parser.add_argument("--self-count", type=int, default=3000000)
    parser.add_argument("--cpu", type=int, required=True, help="one allowed logical CPU")
    parser.add_argument("--output", required=True, help="new JSON output path; never overwrite")
    args = parser.parse_args()
    available_affinity = sorted(os.sched_getaffinity(0))
    if args.cpu not in available_affinity:
        raise ValueError(f"CPU {args.cpu} is not in allowed affinity {available_affinity}")
    os.sched_setaffinity(0, {args.cpu})
    out = Path(args.output).resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    with out.open("x", encoding="utf-8") as f:
        json.dump({"status": "started"}, f)
    cpuinfo = Path("/proc/cpuinfo").read_text(encoding="utf-8")
    model_match = re.search(r"^model name\s*:\s*(.+)$", cpuinfo, re.MULTILINE)
    data = {
        "status": "started", "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "args": vars(args), "platform": platform.platform(), "machine": platform.machine(),
        "processor": platform.processor(), "python": sys.version,
        "cpu_model": model_match.group(1) if model_match else None,
        "available_affinity": available_affinity, "active_affinity": sorted(os.sched_getaffinity(0)),
        "git_head": command_output(["git", "rev-parse", "HEAD"]),
        "compiler": command_output([args.compiler, "--version"]),
        "time_version": command_output(["/usr/bin/time", "--version"]),
        "source_sha256": {f: sha(HERE / f) for f in FILES},
        "commands": [], "tests": [], "samples": [],
    }

    def save():
        out.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

    def run(argv):
        result = command_output([str(x) for x in argv])
        data["commands"].append(result)
        save()
        if result.get("returncode") != 0:
            raise RuntimeError(f"command failed: {argv}")
        return result

    try:
        if args.runs < 1 or args.warmups < 0 or args.n < 2 or args.count < 1 or args.self_count < 1:
            raise ValueError("invalid run count, warmup, n or count")
        if not Path("/usr/bin/time").exists():
            raise RuntimeError("GNU /usr/bin/time required for peak RSS")
        build = out.parent / (out.stem + "-bin")
        build.mkdir(exist_ok=False)
        binaries = {}
        for variant in ("baseline", "candidate"):
            binaries[variant] = {}
            for name in ("test", "benchmark"):
                binary = build / (variant + "-" + name)
                flags = [f"-std={args.std}", "-O2"] + (["-DNDEBUG"] if name == "benchmark" else [])
                source = "test_pinned.cpp" if name == "test" else "benchmark.cpp"
                run([args.compiler, *flags, "-I", HERE / variant, HERE / source, "-o", binary])
                binaries[variant][name] = binary
            for seed in (1, 2, 3, 20261118, 18446744073709551615):
                result = run([binaries[variant]["test"], str(seed)])
                data["tests"].append({"variant": variant, "seed": seed, "result": result})
                save()
        data["binary_sha256"] = {v: {n: sha(p) for n, p in items.items()} for v, items in binaries.items()}
        order_rng = random.Random(args.seed)
        for mode in ("chain", "branch", "self", "deep"):
            mode_count = args.self_count if mode == "self" else args.count
            expected = None
            for iteration in range(-args.warmups, args.runs):
                variants = ["baseline", "candidate"]
                order_rng.shuffle(variants)
                for variant in variants:
                    rss_file = build / "rss.txt"
                    argv = ["/usr/bin/time", "-f", "%M", "-o", rss_file,
                            binaries[variant]["benchmark"], mode, args.n, mode_count, args.seed]
                    started = datetime.datetime.now(datetime.timezone.utc).isoformat()
                    before = time.perf_counter()
                    result = run(argv)
                    elapsed = time.perf_counter() - before
                    values = json.loads(result["stdout"])
                    rss_raw = rss_file.read_text(encoding="utf-8").strip()
                    if not re.fullmatch(r"[0-9]+", rss_raw):
                        raise ValueError(f"invalid RSS: {rss_raw!r}")
                    signature = (values["checksum"], values["versions"], values["merges"])
                    if expected is None:
                        expected = signature
                    elif signature != expected:
                        raise ValueError(f"checksum mismatch: {signature} vs {expected}")
                    data["samples"].append({"variant": variant, "mode": mode, "iteration": iteration,
                                             "warmup": iteration < 0, "wall_s": elapsed, "started_utc": started,
                                             "count": mode_count,
                                             "peak_rss_kib": int(rss_raw), "result": values})
                    save()
        data["summary"] = {}
        for mode in ("chain", "branch", "self", "deep"):
            data["summary"][mode] = {}
            for variant in ("baseline", "candidate"):
                rows = [r for r in data["samples"] if r["mode"] == mode and r["variant"] == variant and not r["warmup"]]
                data["summary"][mode][variant] = {
                    key: {"median": statistics.median(vals), "min": min(vals)}
                    for key, vals in {
                        "merge_ms": [r["result"]["merge_ms"] for r in rows],
                        "same_ms": [r["result"]["same_ms"] for r in rows],
                        "peak_rss_kib": [r["peak_rss_kib"] for r in rows],
                    }.items()
                }
        data["status"] = "complete"
    except Exception as e:
        data["status"] = "failed"
        data["error"] = repr(e)
        raise
    finally:
        data["finished_utc"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
        save()


if __name__ == "__main__":
    main()
