#!/usr/bin/env python3
"""Compile supported headers, verify programs, and randomized tests.

This is intentionally independent of verification-helper.  It provides a fast
preflight check that works with multiple compilers and language standards.
"""

from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import time


ROOT = Path(__file__).resolve().parents[1]
CATALOG = ROOT / ".verify-helper/docs/static/_data/libraries.yml"


@dataclass
class CompileResult:
    name: str
    command: list[str]
    elapsed: float
    returncode: int
    stdout: str
    stderr: str
    executable: Path | None = None


def discover_headers() -> list[Path]:
    """Return documented headers plus supported compatibility entry points."""
    catalog_paths = re.findall(r"^  path: (.+)$", CATALOG.read_text(), re.MULTILINE)
    paths = {ROOT / path for path in catalog_paths}
    for compatibility_header in ("blueberry/all.hpp", "blueberry/fps.hpp"):
        path = ROOT / compatibility_header
        if path.exists():
            paths.add(path)
    missing = sorted(path for path in paths if not path.is_file())
    if missing:
        raise RuntimeError("catalog references missing headers: " + ", ".join(map(str, missing)))
    return sorted(paths)


def discover_sources(directory: str) -> list[Path]:
    root = ROOT / directory
    return sorted(path for path in root.rglob("*.cpp") if path.is_file()) if root.exists() else []


def relative(path: Path) -> str:
    return path.resolve().relative_to(ROOT).as_posix()


def compiler_flags(compiler: str, standard: str) -> list[str]:
    return [
        compiler,
        *shlex.split(os.environ.get("CPPFLAGS", "")),
        f"-std={standard}",
        "-O2",
        "-pipe",
        "-Wall",
        "-Wextra",
        "-Wshadow",
        "-Werror",
        *shlex.split(os.environ.get("CXXFLAGS", "")),
        "-I",
        str(ROOT),
    ]


def compile_one(
    path: Path,
    kind: str,
    output: Path,
    compiler: str,
    standard: str,
    timeout: float,
) -> CompileResult:
    name = relative(path)
    executable = output / re.sub(r"[^A-Za-z0-9_.-]", "_", name)
    command = [*compiler_flags(compiler, standard), "-o", str(executable)]
    source = None
    if kind == "header":
        command.extend(["-x", "c++", "-"])
        source = f'#include "{name}"\n#include "{name}"\nint main() {{}}\n'
    else:
        command.append(str(path))
    started = time.perf_counter()
    try:
        completed = subprocess.run(
            command,
            input=source,
            text=True,
            capture_output=True,
            timeout=timeout,
            cwd=ROOT,
            check=False,
        )
        return CompileResult(
            name=name,
            command=command,
            elapsed=time.perf_counter() - started,
            returncode=completed.returncode,
            stdout=completed.stdout,
            stderr=completed.stderr,
            executable=executable if completed.returncode == 0 else None,
        )
    except subprocess.TimeoutExpired as error:
        return CompileResult(
            name=name,
            command=command,
            elapsed=time.perf_counter() - started,
            returncode=124,
            stdout=error.stdout or "",
            stderr=(error.stderr or "") + f"\ncompile timed out after {timeout:g} seconds",
        )


def print_failure(result: CompileResult, title: str) -> None:
    message = (result.stderr or result.stdout or "compiler exited without output").strip()
    print(f"FAIL {result.name} ({result.elapsed:.2f}s)", file=sys.stderr)
    print("command: " + shlex.join(result.command), file=sys.stderr)
    print(message, file=sys.stderr)
    if os.environ.get("GITHUB_ACTIONS") == "true":
        escaped = message.replace("%", "%25").replace("\r", "%0D").replace("\n", "%0A")
        print(f"::error file={result.name},title={title}::{escaped}", file=sys.stderr)


def compile_many(
    paths: list[Path],
    kind: str,
    output: Path,
    args: argparse.Namespace,
) -> tuple[list[CompileResult], list[CompileResult]]:
    with ThreadPoolExecutor(max_workers=args.jobs) as executor:
        results = list(
            executor.map(
                lambda path: compile_one(
                    path, kind, output, args.compiler, args.standard, args.compile_timeout
                ),
                paths,
            )
        )
    passed = [result for result in results if result.returncode == 0]
    failed = [result for result in results if result.returncode != 0]
    for result in passed:
        print(f"PASS {result.name} ({result.elapsed:.2f}s)")
    for result in failed:
        print_failure(result, f"{kind} compile failed")
    return passed, failed


def run_random_tests(
    paths: list[Path], output: Path, args: argparse.Namespace
) -> tuple[int, list[str]]:
    if not paths:
        print("SKIP random: no tests/random/**/*.cpp files")
        return 0, []

    compiled, failures = compile_many(paths, "random", output, args)
    messages: list[str] = []
    for result in compiled:
        assert result.executable is not None
        for offset in range(args.random_runs):
            seed = args.random_seed + offset
            environment = dict(os.environ, BLUEBERRY_RANDOM_SEED=str(seed))
            command = [str(result.executable), str(seed)]
            started = time.perf_counter()
            try:
                completed = subprocess.run(
                    command,
                    text=True,
                    capture_output=True,
                    timeout=args.random_timeout,
                    cwd=ROOT,
                    env=environment,
                    check=False,
                )
                elapsed = time.perf_counter() - started
                if completed.returncode != 0:
                    detail = (completed.stderr or completed.stdout or "test exited without output").strip()
                    message = (
                        f"FAIL {result.name} seed={seed} ({elapsed:.2f}s)\n"
                        f"rerun: RANDOM_SEED={seed} RANDOM_RUNS=1 make random-test\n{detail}"
                    )
                    print(message, file=sys.stderr)
                    messages.append(message)
                    break
            except subprocess.TimeoutExpired:
                message = (
                    f"FAIL {result.name} seed={seed}: timed out after "
                    f"{args.random_timeout:g} seconds\n"
                    f"rerun: RANDOM_SEED={seed} RANDOM_RUNS=1 make random-test"
                )
                print(message, file=sys.stderr)
                messages.append(message)
                break
        else:
            print(
                f"PASS {result.name} ({args.random_runs} seeds: "
                f"{args.random_seed}..{args.random_seed + args.random_runs - 1})"
            )
    return len(failures) + len(messages), messages


def write_summary(
    args: argparse.Namespace,
    header_counts: tuple[int, int] | None,
    verify_counts: tuple[int, int] | None,
    random_counts: tuple[int, int] | None,
) -> None:
    destination = os.environ.get("GITHUB_STEP_SUMMARY")
    if not destination:
        return
    rows = []
    for label, counts in (
        ("Standalone headers", header_counts),
        ("Verify compile", verify_counts),
        ("Random tests", random_counts),
    ):
        if counts is not None:
            rows.append(f"| {label} | {counts[0]} | {counts[1]} |")
    with Path(destination).open("a") as stream:
        stream.write(
            f"### C++ checks: `{args.compiler}` / `{args.standard}`\n\n"
            "| Check | Passed | Failed |\n| --- | ---: | ---: |\n"
            + "\n".join(rows)
            + "\n\n"
        )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=("compile", "headers", "verify", "random"))
    parser.add_argument("paths", nargs="*", type=Path, help="optional random-test source paths")
    parser.add_argument("--compiler", default=os.environ.get("CXX", "g++"))
    parser.add_argument("--standard", default="gnu++20")
    parser.add_argument("--jobs", type=int, default=2)
    parser.add_argument("--compile-timeout", type=float, default=120)
    parser.add_argument("--random-runs", type=int, default=20)
    parser.add_argument("--random-seed", type=int, default=1)
    parser.add_argument("--random-timeout", type=float, default=5)
    args = parser.parse_args()
    if args.jobs < 1 or args.compile_timeout <= 0 or args.random_runs < 1 or args.random_timeout <= 0:
        parser.error("jobs, timeouts, and random-runs must be positive")
    if args.paths and args.mode != "random":
        parser.error("explicit paths are supported only in random mode")
    return args


def main() -> int:
    args = parse_args()
    if shutil.which(args.compiler) is None:
        print(f"compiler not found: {args.compiler}", file=sys.stderr)
        return 2

    version = subprocess.check_output([args.compiler, "--version"], text=True).splitlines()[0]
    print(f"Compiler: {version}")
    print(f"Standard: {args.standard}")
    header_counts = verify_counts = random_counts = None
    failed = 0
    with tempfile.TemporaryDirectory(prefix="blueberry-cpp-check-") as temporary:
        output = Path(temporary)
        if args.mode in ("compile", "headers"):
            passed, failures = compile_many(discover_headers(), "header", output, args)
            header_counts = (len(passed), len(failures))
            failed += len(failures)
        if args.mode in ("compile", "verify"):
            passed, failures = compile_many(discover_sources("verify"), "verify", output, args)
            verify_counts = (len(passed), len(failures))
            failed += len(failures)
        if args.mode == "random":
            paths = [path.resolve() for path in args.paths] if args.paths else discover_sources("tests/random")
            random_failures, _ = run_random_tests(paths, output, args)
            random_counts = (len(paths) - random_failures, random_failures)
            failed += random_failures

    write_summary(args, header_counts, verify_counts, random_counts)
    print(f"Summary: {failed} failure(s)")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
