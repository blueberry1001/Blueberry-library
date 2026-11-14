#!/usr/bin/env python3
"""Recheck the slowest current verification cases in a separate CI job.

This is a triage signal, not a judge verdict or a required performance gate.
"""
import argparse
from collections import Counter
import json
import math
import os
from pathlib import Path
import signal
import statistics
import subprocess
import sys

if __package__:
    from .audit_verify_limits import audit, git_revision, official_limits
else:
    from audit_verify_limits import audit, git_revision, official_limits


ROOT = Path(__file__).resolve().parents[1]
COMPARISON_KEYS = ("compiler", "flags", "dataset_sha256", "verifier_sha256")


def complete_row(row):
    repeats, runs, cases = row.get("repeat_count"), row.get("runs"), row.get("case_count")
    if (row.get("status") != "passed" or type(repeats) is not int or repeats < 1 or
            type(cases) is not int or cases < 1 or not isinstance(runs, list) or len(runs) != repeats):
        return False
    for run in runs:
        if not isinstance(run, dict) or not isinstance(run.get("log"), str) or not run["log"]:
            return False
        for key in ("solution_seconds", "worst_case_seconds"):
            value = run.get(key)
            if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value < 0:
                return False
    return True


def resolve_case_log(log, root):
    path = Path(log)
    # The initial artifact can come from a different runner workspace.
    if ".verification" in path.parts:
        index = path.parts.index(".verification")
        suffix = path.parts[index + 1:]
        if suffix and ".." not in suffix:
            return root / ".verification" / Path(*suffix)
    return path if path.is_absolute() else root / path


def case_logs_complete(row, root):
    """Do not promote a summary whose original case logs are absent or incomplete."""
    if not complete_row(row):
        return False
    expected_names, maxima = None, []
    try:
        for run in row["runs"]:
            path = resolve_case_log(run["log"], root)
            cases = json.loads(path.read_text(encoding="utf-8"))
            if not isinstance(cases, list) or len(cases) != row["case_count"]:
                return False
            names, elapsed = set(), []
            for case in cases:
                if (not isinstance(case, dict) or not isinstance(case.get("name"), str) or
                        not case["name"] or case["name"] in names or case.get("status") != "AC" or
                        case.get("exitcode") != 0):
                    return False
                value = case.get("elapsed")
                if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value < 0:
                    return False
                names.add(case["name"])
                elapsed.append(value)
            if expected_names is not None and names != expected_names:
                return False
            expected_names = names
            if not math.isclose(max(elapsed), run["worst_case_seconds"], rel_tol=1e-9, abs_tol=1e-9):
                return False
            maxima.append(max(elapsed))
        return math.isclose(statistics.median(maxima), row["worst_case_seconds"], rel_tol=1e-9, abs_tol=1e-9)
    except (OSError, ValueError, KeyError, TypeError):
        return False


def choose_targets(rows, maximum=2):
    """Pick distinct drivers with measured local time at least half the limit."""
    candidates = sorted((r for r in rows if r["assessment"] in ("at_or_above_limit", "review_headroom")),
                        key=lambda r: (-r["ratio"], r["path"], r["environment_index"]))
    result, seen = [], set()
    for row in candidates:
        if row["path"] not in seen:
            result.append(row)
            seen.add(row["path"])
        if len(result) == maximum:
            break
    return result


def comparable(initial, rerun, row, rerun_row):
    """Reject unknown conditions; no comparisons with guessed or stale inputs."""
    if (not initial.get("revision") or initial.get("revision") != rerun.get("revision") or
            not isinstance(initial.get("environment"), dict) or
            initial["environment"] != rerun.get("environment")):
        return False
    if (not complete_row(row) or not complete_row(rerun_row) or rerun_row.get("repeat_count") != 3 or
            row["case_count"] != rerun_row["case_count"]):
        return False
    for key in COMPARISON_KEYS:
        if not row.get(key) or row[key] != rerun_row.get(key):
            return False
    for value in (row.get("worst_case_seconds"), rerun_row.get("worst_case_seconds")):
        if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value <= 0:
            return False
    return True


def failure_status(report_path, root):
    """Separate an observed wrong answer/runtime error from unmeasured failures."""
    try:
        report = json.loads(report_path.read_text(encoding="utf-8"))
        cases_seen = []
        for row in report.get("results", []):
            for run in row.get("runs", []):
                cases = json.loads(resolve_case_log(run["log"], root).read_text(encoding="utf-8"))
                cases_seen.extend(case for case in cases if isinstance(case, dict))
        if any(case.get("status") in ("WA", "RE", "MLE", "OLE") or
               (case.get("status") == "AC" and case.get("exitcode") not in (None, 0))
               for case in cases_seen):
            return "incorrect"
        if any(case.get("status") == "TLE" for case in cases_seen):
            return "timeout"
    except (OSError, ValueError, KeyError, TypeError):
        pass
    return "failed"


def execute(command, cwd, log, timeout):
    """Bound the complete process group and preserve output even on timeout."""
    with log.open("w", encoding="utf-8") as stream:
        process = subprocess.Popen(command, cwd=cwd, stdout=stream, stderr=subprocess.STDOUT,
                                   start_new_session=(os.name == "posix"))
        try:
            return {"exit_code": process.wait(timeout=timeout), "timed_out": False}
        except subprocess.TimeoutExpired:
            if os.name == "posix":
                try:
                    os.killpg(process.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
            else:
                process.kill()
            process.wait()
            return {"exit_code": None, "timed_out": True}


def recheck(initial, root, limits, destination, wall_timeout=360, verify_timeout=300, executor=execute):
    rows = audit(initial, root, limits)
    initial_rows = {(row["path"], row.get("environment_index")): row for row in initial["results"]}
    for row in rows:
        source = initial_rows.get((row["path"], row["environment_index"]))
        if row["assessment"] not in ("unmeasured", "source_changed_or_unknown") and (not source or not case_logs_complete(source, root)):
            row.update(assessment="unmeasured", ratio=None)
    candidates = choose_targets(rows)
    result = {"initial_revision": initial.get("revision"), "initial_succeeded": initial.get("succeeded"),
              "screening_counts": dict(Counter(row["assessment"] for row in rows)),
              "screening": rows, "rechecks": []}
    current_revision, _ = git_revision(root)
    tle = initial.get("environment", {}).get("tle_seconds")
    valid_tle = (not isinstance(tle, bool) and isinstance(tle, (int, float)) and
                 math.isfinite(tle) and tle > 0)
    can_run = current_revision == initial.get("revision") and valid_tle
    for number, candidate in enumerate(candidates, 1):
        item = {"path": candidate["path"], "environment_index": candidate["environment_index"],
                "initial_worst_seconds": candidate["worst_case_seconds"],
                "limit_seconds": candidate["limit"]["seconds"], "initial_limit_ratio": candidate["ratio"],
                "status": "unmeasured"}
        result["rechecks"].append(item)
        if not can_run:
            item["reason"] = "initial revision changed or correctness timeout is unknown"
            continue
        directory = destination / f"recheck-{number}"
        directory.mkdir(parents=True, exist_ok=True)
        command = [sys.executable, str(root / "scripts/verify_with_metrics.py"),
                   "--repeats", "3", "--tle", str(tle), "--timeout", str(verify_timeout),
                   "--output", str(directory.resolve()), candidate["path"]]
        item["command"] = command
        item["log"] = f"recheck-{number}/execution.log"
        try:
            execution = executor(command, root, directory / "execution.log", wall_timeout)
            item.update(execution)
            if execution["timed_out"]:
                item.update(status="timeout", reason="recheck exceeded its wall-time budget")
                continue
            if execution["exit_code"] != 0:
                item.update(status=failure_status(directory / "current.json", root), reason="verification command failed")
                continue
            report_path = directory / "current.json"
            rerun = json.loads(report_path.read_text(encoding="utf-8"))
            original_rows = [r for r in initial["results"] if r["path"] == candidate["path"] and
                             r.get("environment_index") == candidate["environment_index"]]
            rerun_rows = [r for r in rerun.get("results", []) if r["path"] == candidate["path"] and
                          r.get("environment_index") == candidate["environment_index"]]
            if (len(original_rows) != 1 or len(rerun_rows) != 1 or rerun.get("succeeded") is not True or
                    rerun_rows[0].get("repeat_count") != 3 or not case_logs_complete(rerun_rows[0], root)):
                item.update(status="unmeasured", reason="recheck measurement or case logs are incomplete")
                continue
            rerun_row = rerun_rows[0]
            measured = rerun_row["worst_case_seconds"]
            same_conditions = comparable(initial, rerun, original_rows[0], rerun_row)
            differences = []
            if initial.get("revision") != rerun.get("revision"):
                differences.append("revision")
            if initial.get("environment") != rerun.get("environment"):
                differences.append("environment")
            for key in (*COMPARISON_KEYS, "case_count"):
                if not original_rows[0].get(key) or original_rows[0].get(key) != rerun_row.get(key):
                    differences.append(key)
            item.update(status="measured" if same_conditions else "different_conditions",
                        condition_mismatches=differences,
                        repeated_worst_seconds=measured,
                        recheck_revision=rerun.get("revision"), recheck_environment=rerun.get("environment"),
                        recheck_compiler=rerun_row.get("compiler"), recheck_flags=rerun_row.get("flags"),
                        recheck_dataset_sha256=rerun_row.get("dataset_sha256"),
                        recheck_verifier_sha256=rerun_row.get("verifier_sha256"),
                        recheck_report=f"recheck-{number}/current.json")
            if rerun_row.get("verifier_sha256") == original_rows[0].get("verifier_sha256"):
                item["repeated_limit_ratio"] = measured / candidate["limit"]["seconds"]
        except (OSError, ValueError, KeyError, TypeError) as error:
            item.update(status="failed", reason=f"recheck data unavailable: {error}")
    return result


def write_output(result, destination):
    destination.mkdir(parents=True, exist_ok=True)
    (destination / "performance.json").write_text(json.dumps(result, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    lines = ["## Verification performance screening", "",
             "Local maximum-case medians are compared with problem time limits to select rechecks. "
             "Runner timing differs from the judge; this is not an AC/TLE verdict.", "",
             "Screening counts: " + ", ".join(f"{key}={value}" for key, value in sorted(result["screening_counts"].items())), "",
             "| Verify | Initial / limit | Three-run recheck / limit | Status |",
             "| --- | ---: | ---: | --- |"]
    for row in result["rechecks"]:
        repeated = f"{row['repeated_limit_ratio']:.2f}" if row.get("repeated_limit_ratio") is not None else "unmeasured"
        lines.append(f"| {row['path']} | {row['initial_limit_ratio']:.2f} | {repeated} | {row['status']} |")
        print(f"::warning::Performance recheck: {row['path']} initial/TL={row['initial_limit_ratio']:.2f}, status={row['status']}, recheck/TL={repeated}")
    if result.get("error"):
        lines += ["", f"Screening unavailable: {result['error']}"]
    elif not result["rechecks"]:
        lines += ["", "No measured target reached the screening threshold; unmeasured and unknown targets remain unassessed."]
    lines += ["", "Available initial evidence and recheck logs are retained in their respective artifacts.", ""]
    summary = "\n".join(lines)
    (destination / "summary.md").write_text(summary, encoding="utf-8")
    if os.environ.get("GITHUB_STEP_SUMMARY"):
        with Path(os.environ["GITHUB_STEP_SUMMARY"]).open("a", encoding="utf-8") as stream:
            stream.write(summary + "\n")
    print(summary)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--problem-repo", type=Path, required=True)
    parser.add_argument("--expected-problem-revision", help="require this clean official repository revision")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    args.output.mkdir(parents=True, exist_ok=True)
    try:
        initial = json.loads(args.report.read_text(encoding="utf-8"))
        problem_revision, problem_dirty = git_revision(args.problem_repo)
        if args.expected_problem_revision and (problem_revision != args.expected_problem_revision or problem_dirty is not False):
            raise ValueError("official problem checkout is missing, changed, or at an unexpected revision")
        result = recheck(initial, ROOT, official_limits(args.problem_repo), args.output)
        result.update(problem_revision=problem_revision, problem_worktree_dirty=problem_dirty,
                      initial_report=str(args.report), problem_repo=str(args.problem_repo))
    except (OSError, ValueError, KeyError, TypeError) as error:
        result = {"screening_counts": {"unavailable": 1}, "screening": [], "rechecks": [], "error": str(error)}
        print(f"::warning::Verification performance screening unavailable: {error}")
    write_output(result, args.output)
    return 1 if any(row["status"] == "incorrect" for row in result["rechecks"]) else 0


if __name__ == "__main__":
    sys.exit(main())
