#!/usr/bin/env python3
"""Compile and run the local benchmark matrix with interleaved repetitions."""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import fnmatch
import hashlib
import json
from pathlib import Path
import platform
import shutil
import statistics
import subprocess
import sys
import time
from typing import Any, Dict, Iterable, List, Optional, Sequence, Tuple


ROOT = Path(__file__).resolve().parents[1]


def median_absolute_deviation(values: Sequence[float]) -> float:
    center = statistics.median(values)
    return statistics.median(abs(value - center) for value in values)


def slug(text: str) -> str:
    return "".join(char if char.isalnum() or char in "-_" else "-" for char in text)


def load_manifest(path: Path) -> Dict[str, Any]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    if payload.get("schema") != 1:
        raise ValueError("benchmark manifest schema must be 1")
    if not isinstance(payload.get("flag_sets"), dict) or not isinstance(payload.get("targets"), list):
        raise ValueError("benchmark manifest needs flag_sets and targets")
    names = [target.get("name") for target in payload["targets"]]
    if any(not isinstance(name, str) or not name for name in names) or len(names) != len(set(names)):
        raise ValueError("benchmark target names must be non-empty and unique")
    baselines: Dict[str, int] = {}
    for target in payload["targets"]:
        if target.get("baseline"):
            group = target.get("group", target["name"])
            baselines[group] = baselines.get(group, 0) + 1
    if any(count != 1 for count in baselines.values()):
        raise ValueError("each group may have at most one baseline")
    return payload


def compiler_version(compiler: str) -> str:
    command = [compiler, "--version"]
    result = subprocess.run(command, check=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return result.stdout.splitlines()[0]


def cpu_model() -> str:
    cpuinfo = Path("/proc/cpuinfo")
    if cpuinfo.is_file():
        for line in cpuinfo.read_text(encoding="utf-8", errors="replace").splitlines():
            if line.startswith("model name") and ":" in line:
                return line.split(":", 1)[1].strip()
    return platform.processor() or "unknown"


def file_sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def build_matrix(
    manifest: Dict[str, Any], compiler: str, flag_names: Sequence[str], targets: Sequence[Dict[str, Any]],
    build_dir: Path,
) -> List[Dict[str, Any]]:
    build_dir.mkdir(parents=True, exist_ok=True)
    jobs: List[Dict[str, Any]] = []
    for target in targets:
        source = ROOT / target["source"]
        if not source.is_file():
            raise ValueError(f"benchmark source does not exist: {source}")
        for flag_name in flag_names:
            flags = manifest["flag_sets"].get(flag_name)
            if flags is None:
                raise ValueError(f"unknown flag set: {flag_name}")
            binary = build_dir / f"{slug(target['name'])}--{slug(flag_name)}"
            command = [compiler, *flags, "-I", str(ROOT)]
            command += [f"-D{define}" for define in target.get("defines", [])]
            command += [str(source), "-o", str(binary)]
            started = time.perf_counter()
            result = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            elapsed = time.perf_counter() - started
            if result.returncode != 0:
                raise RuntimeError(
                    f"compile failed for {target['name']} / {flag_name}\n"
                    f"command: {' '.join(command)}\n{result.stdout}{result.stderr}"
                )
            jobs.append({
                "target": target,
                "flag_set": flag_name,
                "flags": list(flags),
                "binary": binary,
                "compile_command": command,
                "compile_seconds": elapsed,
                "binary_bytes": binary.stat().st_size,
                "source_sha256": file_sha256(source),
                "samples": [],
            })
    return jobs


def parse_benchmark_output(stdout: str, name: str) -> Dict[str, Any]:
    lines = [line for line in stdout.splitlines() if line.strip()]
    if len(lines) != 1:
        raise RuntimeError(f"{name} must print exactly one non-empty JSON line")
    try:
        payload = json.loads(lines[0])
    except ValueError as error:
        raise RuntimeError(f"{name} printed invalid JSON: {error}") from error
    if "checksum" not in payload:
        raise RuntimeError(f"{name} output has no checksum")
    for key, value in payload.items():
        if key.endswith("_seconds") and (not isinstance(value, (int, float)) or value < 0):
            raise RuntimeError(f"{name} output has invalid {key}")
    return payload


def run_once(job: Dict[str, Any], cpu: Optional[int], timeout: float) -> Dict[str, Any]:
    command = [str(job["binary"]), *map(str, job["target"].get("args", []))]
    if cpu is not None:
        command = ["taskset", "-c", str(cpu), *command]
    started = time.perf_counter()
    result = subprocess.run(
        command, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=timeout
    )
    wall = time.perf_counter() - started
    if result.returncode != 0:
        raise RuntimeError(
            f"benchmark failed for {job['target']['name']} / {job['flag_set']}\n"
            f"command: {' '.join(command)}\n{result.stdout}{result.stderr}"
        )
    payload = parse_benchmark_output(result.stdout, job["target"]["name"])
    payload["wall_seconds"] = wall
    return payload


def rotate(items: Sequence[Dict[str, Any]], amount: int) -> Iterable[Dict[str, Any]]:
    if not items:
        return []
    amount %= len(items)
    return [*items[amount:], *items[:amount]]


def execute_matrix(
    jobs: List[Dict[str, Any]], repeats: int, warmups: int, cpu: Optional[int], timeout: float
) -> None:
    for warmup in range(warmups):
        for job in rotate(jobs, warmup):
            run_once(job, cpu, timeout)
    for repeat in range(repeats):
        # Rotation prevents every repetition from favoring the same first/last target.
        for job in rotate(jobs, repeat):
            job["samples"].append(run_once(job, cpu, timeout))


def validate_checksums(jobs: Sequence[Dict[str, Any]]) -> None:
    expected: Dict[Tuple[str, str], Any] = {}
    for job in jobs:
        checksums = {sample["checksum"] for sample in job["samples"]}
        if len(checksums) != 1:
            raise RuntimeError(f"checksum changed between repetitions for {job['target']['name']}")
        key = (job["target"].get("group", job["target"]["name"]), job["flag_set"])
        checksum = next(iter(checksums))
        if key in expected and expected[key] != checksum:
            raise RuntimeError(f"checksum mismatch inside benchmark group {key[0]} / {key[1]}")
        expected[key] = checksum


def summarize_job(job: Dict[str, Any]) -> Dict[str, Any]:
    numeric_keys = sorted({
        key for sample in job["samples"] for key, value in sample.items()
        if isinstance(value, (int, float)) and not isinstance(value, bool) and key != "checksum"
    })
    summary: Dict[str, Any] = {
        "target": job["target"]["name"],
        "group": job["target"].get("group", job["target"]["name"]),
        "baseline": bool(job["target"].get("baseline")),
        "flag_set": job["flag_set"],
        "flags": job["flags"],
        "args": job["target"].get("args", []),
        "compile_seconds": job["compile_seconds"],
        "binary_bytes": job["binary_bytes"],
        "source_sha256": job["source_sha256"],
        "checksum": job["samples"][0]["checksum"],
        "samples": job["samples"],
    }
    for key in numeric_keys:
        values = [float(sample[key]) for sample in job["samples"] if key in sample]
        summary[f"median_{key}"] = statistics.median(values)
        summary[f"min_{key}"] = min(values)
        summary[f"max_{key}"] = max(values)
        summary[f"mad_{key}"] = median_absolute_deviation(values)
    return summary


def add_ratios(rows: List[Dict[str, Any]]) -> None:
    baselines = {
        (row["group"], row["flag_set"]): row for row in rows if row["baseline"]
    }
    for row in rows:
        baseline = baselines.get((row["group"], row["flag_set"]))
        reference = baseline and baseline.get("median_query_seconds")
        value = row.get("median_query_seconds")
        row["query_ratio_to_baseline"] = value / reference if reference and value is not None else None


def format_seconds(value: Optional[float]) -> str:
    return "—" if value is None else f"{value:.6f}"


def write_report(report: Dict[str, Any], output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "results.json").write_text(
        json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    lines = [
        "# Local benchmark results",
        "",
        f"Generated: {report['generated_at']} / revision: `{report['revision']}`",
        "",
        "Times are seconds. Medians and median absolute deviations use interleaved repetitions.",
        "The ratio compares query time with the current Blueberry implementation under identical flags.",
        "It is not a ratio to the Library Checker leaderboard.",
        "",
        "| target | flags | build median | query median | query MAD | wall median | structure | ratio |",
        "| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |",
    ]
    for row in report["results"]:
        ratio = row["query_ratio_to_baseline"]
        structure = row.get("median_estimated_structure_bytes")
        lines.append(
            f"| {row['target']} | {row['flag_set']} | {format_seconds(row.get('median_build_seconds'))} "
            f"| {format_seconds(row.get('median_query_seconds'))} "
            f"| {format_seconds(row.get('mad_query_seconds'))} "
            f"| {format_seconds(row.get('median_wall_seconds'))} "
            f"| {int(structure) if structure is not None else '—'} B "
            f"| {f'{ratio:.3f}x' if ratio is not None else '—'} |"
        )
    lines += ["", "## Environment", "", "```json", json.dumps(report["environment"], indent=2), "```", ""]
    markdown = "\n".join(lines)
    (output_dir / "report.md").write_text(markdown, encoding="utf-8")
    print(markdown)


def parse_args(argv: Sequence[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, default=ROOT / "benchmark/manifest.json")
    parser.add_argument("--compiler")
    parser.add_argument("--flag-set", action="append", dest="flag_sets")
    parser.add_argument("--target", action="append", help="target glob; repeat to combine patterns")
    parser.add_argument("--repeats", type=int)
    parser.add_argument("--warmups", type=int)
    parser.add_argument("--cpu", type=int, help="pin child processes with taskset")
    parser.add_argument("--timeout", type=float)
    parser.add_argument("--output", type=Path, default=ROOT / ".benchmark/results/latest")
    parser.add_argument("--build-dir", type=Path, default=ROOT / ".benchmark/bin")
    return parser.parse_args(argv)


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    manifest = load_manifest(args.manifest)
    defaults = manifest.get("defaults", {})
    compiler = args.compiler or defaults.get("compiler", "g++")
    flag_names = args.flag_sets or defaults.get("flag_sets", [])
    repeats = args.repeats if args.repeats is not None else int(defaults.get("repeats", 7))
    warmups = args.warmups if args.warmups is not None else int(defaults.get("warmups", 1))
    timeout = args.timeout if args.timeout is not None else float(defaults.get("timeout_seconds", 30))
    if repeats < 1 or warmups < 0 or timeout <= 0:
        raise SystemExit("repeats must be positive; warmups non-negative; timeout positive")
    if args.cpu is not None and shutil.which("taskset") is None:
        raise SystemExit("--cpu requires taskset")

    patterns = args.target or ["*"]
    targets = [
        target for target in manifest["targets"]
        if any(fnmatch.fnmatchcase(target["name"], pattern) for pattern in patterns)
    ]
    if not targets:
        raise SystemExit("no benchmark targets matched")

    jobs = build_matrix(manifest, compiler, flag_names, targets, args.build_dir)
    execute_matrix(jobs, repeats, warmups, args.cpu, timeout)
    validate_checksums(jobs)
    rows = [summarize_job(job) for job in jobs]
    add_ratios(rows)
    revision = subprocess.run(
        ["git", "rev-parse", "HEAD"], cwd=ROOT, check=True, text=True, stdout=subprocess.PIPE
    ).stdout.strip()
    report = {
        "schema": 1,
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "revision": revision,
        "manifest_sha256": file_sha256(args.manifest),
        "environment": {
            "cpu": cpu_model(),
            "os": platform.platform(),
            "python": platform.python_version(),
            "compiler": compiler_version(compiler),
            "cpu_affinity": args.cpu,
            "repeats": repeats,
            "warmups": warmups,
            "interleaved": True,
        },
        "results": rows,
    }
    write_report(report, args.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
