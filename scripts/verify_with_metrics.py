#!/usr/bin/env python3
"""Run verification-helper's normal checker flow, with serial/repeated timing.

Integration targets verification-helper 5.6.0 and the PyPI release oj 11.5.1.
No submission API, checker replacement, or timestamp deletion is involved.
"""
import argparse
import hashlib
import importlib.metadata
import json
import math
import os
from pathlib import Path
import platform
import statistics
import subprocess
import sys
import tempfile
import time
from datetime import datetime, timezone
from unittest.mock import patch


def digest_files(paths, root):
    digest = hashlib.sha256()
    for path in sorted(paths):
        digest.update(str(path.relative_to(root)).encode())
        digest.update(b"\0")
        with path.open("rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(chunk)
        digest.update(b"\0")
    return digest.hexdigest()


def read_samples(payload, expected_names):
    """Validate oj's structured log; missing/invalid measurements are never zero."""
    if not isinstance(payload, list) or not payload:
        raise ValueError("oj returned an empty or invalid timing log")
    names = [case["testcase"]["name"] for case in payload]
    if sorted(names) != sorted(expected_names):
        raise ValueError("oj timing log does not match the complete test set")
    samples = []
    for case in payload:
        elapsed = case.get("elapsed")
        if isinstance(elapsed, bool) or not isinstance(elapsed, (int, float)):
            raise ValueError("oj did not report a numeric elapsed time")
        if not math.isfinite(elapsed) or elapsed < 0:
            raise ValueError("oj reported an invalid elapsed time")
        samples.append({
            "name": case["testcase"]["name"],
            "status": case["status"],
            "elapsed": elapsed,
            "exitcode": case["exitcode"],
            "memory": case.get("memory"),
        })
    return samples


def summarize(repeats):
    totals = [sum(case["elapsed"] for case in run) for run in repeats]
    maxima = [max(case["elapsed"] for case in run) for run in repeats]
    return {
        "solution_seconds": statistics.median(totals),
        "worst_case_seconds": statistics.median(maxima),
        "min_solution_seconds": min(totals),
        "max_solution_seconds": max(totals),
        "case_count": len(repeats[0]),
    }


def compare(current, previous, environment, baseline):
    if current["status"] != "passed":
        return None, "未完了 / 失敗"
    if not previous or not baseline:
        return None, "初回（基準なし）"
    if previous["status"] != "passed":
        return None, "比較不可（基準が未完了）"
    if not environment.get("cpu") or baseline.get("schema") != 1:
        return None, "比較不可（環境情報不足）"
    if environment != baseline.get("environment"):
        return None, "比較不可（実行環境が異なる）"
    for key in ("compiler", "flags", "dataset_sha256", "verifier_sha256", "repeat_count"):
        if key not in current or current[key] != previous.get(key):
            return None, "比較不可（コンパイラ・テスト・計測条件が異なる）"
    old = previous.get("solution_seconds")
    if not old or old <= 0:
        return None, "比較不可（基準時間なし）"
    delta = 100 * (current["solution_seconds"] / old - 1)
    return delta, f"{delta:+.2f}%（負=短縮）"


def environment_info():
    cpu = platform.processor()
    cpuinfo = Path("/proc/cpuinfo")
    if cpuinfo.exists():
        for line in cpuinfo.read_text().splitlines():
            if line.startswith("model name"):
                cpu = line.split(":", 1)[1].strip()
                break
    versions = {}
    for package in ("online-judge-verify-helper", "online-judge-tools", "online-judge-api-client"):
        try:
            versions[package] = importlib.metadata.version(package)
        except importlib.metadata.PackageNotFoundError:
            versions[package] = "unavailable"
    return {
        "cpu": cpu,
        "os": platform.platform(),
        "python": platform.python_version(),
        "tools": versions,
        "method": "oj-elapsed-log-v1",
        "recorder_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "jobs": 1,
    }


def seconds(value):
    return "—" if value is None else f"{value:.6f} s"


def write_report(report, baseline, destination):
    previous = {(r["path"], r["environment_index"]): r for r in (baseline or {}).get("results", [])}
    for row in report["results"]:
        old = previous.get((row["path"], row["environment_index"]))
        row["delta_percent"], row["comparison"] = compare(row, old, report["environment"], baseline)
    lines = [
        "# Verification timings",
        "",
        f"Commit: {report['revision']} / UTC: {report['generated_at']}",
        f"Baseline: {report.get('baseline_revision') or 'なし（初回）'}",
        "",
        "単位は秒。直列実行・各ケースの solution elapsed 合計の中央値。コンパイル・DL・checkerは合計から除外。",
        "プロセス起動とI/Oは含みます。Yosupo提出画面の時間ではありません。共有runnerの小さな差は誤差として扱ってください。",
        "",
        "| Verify / 環境番号 | 状態 | コンパイル | 実行合計（中央値） | 最大ケース（中央値） | 基準との差 |",
        "| --- | --- | ---: | ---: | ---: | --- |",
    ]
    for row in report["results"]:
        lines.append(f"| {row['path']} / {row['environment_index']} | {row['status']} | "
                     f"{seconds(row.get('compile_seconds'))} | {seconds(row.get('solution_seconds'))} | "
                     f"{seconds(row.get('worst_case_seconds'))} | {row['comparison']} |")
    lines += ["", "## Environment", "", "~~~json", json.dumps(report["environment"], ensure_ascii=False, indent=2), "~~~", ""]
    destination.mkdir(parents=True, exist_ok=True)
    (destination / "current.json").write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
    markdown = "\n".join(lines)
    (destination / "report.md").write_text(markdown)
    if os.environ.get("GITHUB_STEP_SUMMARY"):
        with Path(os.environ["GITHUB_STEP_SUMMARY"]).open("a") as stream:
            stream.write(markdown)
    print(markdown)


class Recorder:
    def __init__(self, report, output, repeats):
        self.report = report
        self.repeats = repeats
        self.logs = Path(tempfile.mkdtemp(prefix="run-", dir=output))
        self.active = None

    def verify(self, original, path, **kwargs):
        name = path.as_posix()
        self.active = next(row for row in self.report["results"] if row["path"] == name)
        start_index = self.report["results"].index(self.active)
        start = time.perf_counter()
        try:
            result = original(path, **kwargs)
            if result is None:
                self.active["status"] = "ignored"
            elif not result:
                self.active["status"] = "failed"
            elif self.active["status"] != "passed":
                raise RuntimeError("verification passed without a measured C++ test")
            return result
        except BaseException:
            self.active["status"] = "failed"
            raise
        finally:
            self.report["results"][start_index]["verify_wall_seconds"] = time.perf_counter() - start
            self.active = None

    def compile(self, original, environment, path, **kwargs):
        if self.active is None:
            return original(environment, path, **kwargs)
        if "compiler" in self.active:
            self.active = {
                "path": path.as_posix(),
                "environment_index": self.active["environment_index"] + 1,
                "status": "not_run",
            }
            self.report["results"].append(self.active)
        row = self.active
        row.update({
            "compiler": subprocess.check_output([str(environment.CXX), "--version"], text=True).strip(),
            "flags": environment.CXXFLAGS,
            "verifier_sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
            "repeat_count": self.repeats,
            "runs": [],
        })
        start = time.perf_counter()
        try:
            return original(environment, path, **kwargs)
        finally:
            row["compile_seconds"] = time.perf_counter() - start

    def execute(self, original, command):
        if command[:2] != ["oj", "test"]:
            return original(command)
        if self.active is None:
            raise RuntimeError("oj test called outside a measured verify")
        row = self.active
        directory = Path(command[command.index("-d") + 1])
        inputs = sorted(directory.glob("*.in"))
        names = [path.stem for path in inputs]
        if not names:
            raise RuntimeError("no test inputs found")
        row["dataset_sha256"] = digest_files(
            [*inputs, *directory.glob("*.out")], directory)
        # The upstream helper omits -j for jobs=1: oj then runs serially.
        if "-j" in command or "--jobs" in command:
            raise RuntimeError("benchmarking requires serial execution")
        identifier = hashlib.sha256(row["path"].encode()).hexdigest()[:16]
        all_samples = []
        for repeat in range(self.repeats):
            log = self.logs / f"{identifier}-{row['environment_index']}-{repeat + 1}.json"
            timed_command = [*command, "--log-file", str(log)]
            print(f"::group::{row['path']} measurement {repeat + 1}/{self.repeats}", flush=True)
            start = time.perf_counter()
            try:
                completed = subprocess.run(timed_command, check=False)
            finally:
                wall = time.perf_counter() - start
                print("::endgroup::", flush=True)
            if not log.exists():
                raise RuntimeError(f"oj produced no timing log (exit {completed.returncode})")
            samples = read_samples(json.loads(log.read_text()), names)
            # Do not retain potentially huge solution output in artifacts.
            log.write_text(json.dumps(samples, ensure_ascii=False, indent=2) + "\n")
            row["runs"].append({
                "log": str(log),
                "oj_wall_seconds": wall,
                "solution_seconds": sum(case["elapsed"] for case in samples),
                "worst_case_seconds": max(case["elapsed"] for case in samples),
            })
            if completed.returncode or any(c["status"] != "AC" or c["exitcode"] != 0 for c in samples):
                raise RuntimeError("verification failed; partial timings are not a performance result")
            all_samples.append(samples)
        row.update(summarize(all_samples))
        row["status"] = "passed"


def discover_verification_files(root=Path("verify")):
    """Never discover Jekyll redirects or copied sources in generated output."""
    return sorted(path for path in root.rglob("*.test.cpp") if path.is_file())


def run_with_helper(args, report):
    # These imports stay lazy so report/aggregation unit tests need only stdlib.
    import onlinejudge_verify.config as config
    import onlinejudge_verify.languages.cplusplus as cpp
    import onlinejudge_verify.marker as marker_module
    import onlinejudge_verify.verify as verify
    from logging import INFO, basicConfig

    basicConfig(level=INFO)
    config.set_config_path(Path(".verify-helper/config.toml"))
    paths = [Path(p) for p in args.paths] or discover_verification_files()
    paths = sorted(set(p.resolve().relative_to(Path.cwd()) for p in paths))
    if not paths:
        raise RuntimeError("no verification files found")
    report["results"] = [{"path": p.as_posix(), "environment_index": 0, "status": "not_run"} for p in paths]
    recorder = Recorder(report, args.output, args.repeats)
    original_verify, original_compile, original_exec = verify.verify_file, cpp.CPlusPlusLanguageEnvironment.compile, verify.exec_command
    with patch.object(verify, "verify_file", lambda path, **kw: recorder.verify(original_verify, path, **kw)), \
         patch.object(cpp.CPlusPlusLanguageEnvironment, "compile", lambda env, path, **kw: recorder.compile(original_compile, env, path, **kw)), \
         patch.object(verify, "exec_command", lambda cmd: recorder.execute(original_exec, cmd)), \
         patch.object(marker_module.VerificationMarker, "is_verified", return_value=False):
        # Force actual measurement even if timestamp cache says "verified".
        # The normal marker still records AC/WA for generated documentation.
        with marker_module.get_verification_marker(jobs=1) as marker:
            summary = verify.main(paths, marker=marker, jobs=1, tle=args.tle, timeout=args.timeout)
            summary.show()
    return summary.succeeded() and all(row["status"] in ("passed", "ignored") for row in report["results"])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("paths", nargs="*", help="verify files; omitted means all")
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--tle", type=float, default=60)
    parser.add_argument("--timeout", type=float, default=1500)
    parser.add_argument("--output", type=Path, default=Path(".verification"))
    parser.add_argument("--baseline", type=Path, default=Path(".verification/baseline.json"))
    parser.add_argument("--save-baseline", action="store_true", help="replace baseline only after a successful run")
    args = parser.parse_args()
    if args.repeats < 1 or args.tle <= 0 or args.timeout <= 0:
        parser.error("repeats, tle and timeout must be positive")
    args.output.mkdir(parents=True, exist_ok=True)
    baseline = None
    if args.baseline.exists():
        try:
            baseline = json.loads(args.baseline.read_text())
        except (ValueError, OSError) as error:
            print(f"Baseline unavailable: {error}", file=sys.stderr)
    report = {
        "schema": 1,
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "revision": subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip(),
        "baseline_revision": (baseline or {}).get("revision"),
        "environment": environment_info(),
        "results": [],
        "succeeded": False,
    }
    report["environment"]["tle_seconds"] = args.tle
    try:
        report["succeeded"] = run_with_helper(args, report)
    finally:
        # Compile errors, WA, and exceptions still leave a usable CI artifact.
        write_report(report, baseline, args.output)
    if report["succeeded"] and args.save_baseline:
        args.baseline.parent.mkdir(parents=True, exist_ok=True)
        args.baseline.write_text((args.output / "current.json").read_text())
    return 0 if report["succeeded"] else 1


if __name__ == "__main__":
    sys.exit(main())
