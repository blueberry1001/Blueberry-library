import argparse
import datetime
import hashlib
import json
import os
import platform
import subprocess
from pathlib import Path

HERE = Path(__file__).resolve().parent


def digest(data):
    return hashlib.sha256(data).hexdigest()


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--cpu", required=True, type=int)
    p.add_argument("--compiler", default="g++")
    p.add_argument("--output", required=True)
    args = p.parse_args()
    available = sorted(os.sched_getaffinity(0))
    if args.cpu not in available:
        raise ValueError("CPU not in allowed affinity")
    os.sched_setaffinity(0, {args.cpu})
    out = Path(args.output).resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    with out.open("x", encoding="utf-8") as f:
        json.dump({"status": "started"}, f)
    build = out.parent / (out.stem + "-bin")
    data = {"status": "started", "args": vars(args), "affinity": sorted(os.sched_getaffinity(0)),
            "platform": platform.platform(), "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
            "commands": [], "comparisons": [], "node_stats": []}

    def save():
        out.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")

    def run(argv, large=False, env=None):
        argv = [str(x) for x in argv]
        result = subprocess.run(argv, capture_output=True, env=env)
        data["commands"].append({"argv": argv, "returncode": result.returncode,
                                 "stdout_sha256": digest(result.stdout),
                                 "stdout": None if large else result.stdout.decode("utf-8", "replace"),
                                 "stderr": result.stderr.decode("utf-8", "replace")})
        save()
        if result.returncode:
            raise RuntimeError(f"command failed: {argv}")
        return result.stdout

    try:
        build.mkdir(exist_ok=False)
        run([args.compiler, "--version"])
        run(["git", "rev-parse", "HEAD"])
        sources = [HERE / "test_pinned.cpp", HERE / "direct_compare.cpp", HERE / "instrumented/benchmark-stats.cpp", Path(__file__)]
        for variant in ("baseline", "candidate"):
            sources += list((HERE / variant / "blueberry/data-structure").glob("*.hpp"))
            sources += list((HERE / "instrumented" / variant / "blueberry/data-structure").glob("*.hpp"))
        data["source_sha256"] = {str(path.relative_to(HERE)): digest(path.read_bytes()) for path in sources}
        binaries = {}
        for variant in ("baseline", "candidate"):
            binaries[variant] = {}
            for name, source, flags, include in (
                ("direct", "direct_compare.cpp", ["-O2", "-DNDEBUG"], HERE / variant),
                ("san", "test_pinned.cpp", ["-O1", "-g", "-fsanitize=address,undefined", "-fno-omit-frame-pointer"], HERE / variant),
                ("stats", "instrumented/benchmark-stats.cpp", ["-O2", "-DNDEBUG"], HERE / "instrumented" / variant),
            ):
                binary = build / f"{variant}-{name}"
                run([args.compiler, "-std=gnu++20", *flags, "-I", include, HERE / source, "-o", binary])
                binaries[variant][name] = binary
            env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0")
            for seed in (1, 2, 3, 20261118, 18446744073709551615):
                run([binaries[variant]["san"], seed], env=env)
        data["binary_sha256"] = {v: {k: digest(path.read_bytes()) for k, path in entries.items()} for v, entries in binaries.items()}
        for seed in (1, 2, 3, 20261118, 18446744073709551615):
            old = run([binaries["baseline"]["direct"], seed], large=True)
            new = run([binaries["candidate"]["direct"], seed], large=True)
            comparison = {"seed": seed, "equal": old == new, "old_bytes": len(old), "new_bytes": len(new),
                          "old_sha256": digest(old), "new_sha256": digest(new)}
            data["comparisons"].append(comparison)
            save()
            if old != new:
                raise ValueError(f"direct output mismatch seed={seed}")
        for mode in ("chain", "branch", "self", "deep"):
            results = {}
            for variant in ("baseline", "candidate"):
                results[variant] = json.loads(run([binaries[variant]["stats"], mode, 131072, 200000, 20261118]))
            old, new = results["baseline"], results["candidate"]
            for key in ("success", "same", "expected_old_nodes", "expected_new_nodes", "checksum", "versions", "merges", "shared_histogram"):
                if old[key] != new[key]:
                    raise ValueError(f"instrumented mismatch {mode} {key}")
            if old["observed_nodes"] != old["expected_old_nodes"] or new["observed_nodes"] != old["expected_new_nodes"]:
                raise ValueError(f"node count mismatch: {mode}")
            data["node_stats"].append({"mode": mode, "baseline": old, "candidate": new})
            save()
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
