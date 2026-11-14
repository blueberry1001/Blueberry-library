#!/usr/bin/env python3
"""Compare recorded local case timings with Library Checker problem limits.

The comparison is a screening aid, not a prediction of the judge verdict.
The official problem repository is supplied locally to avoid hidden network access.
"""
import argparse
import hashlib
import json
import math
from pathlib import Path
import re
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
PROBLEM = re.compile(r'^\s*#\s*define\s+PROBLEM\s+"https?://judge\.yosupo\.jp/problem/([a-zA-Z0-9_]+)"', re.M)
LIMIT = re.compile(r'^\s*timelimit\s*=\s*([0-9]+(?:\.[0-9]+)?)\s*(?:#.*)?$')


def official_limits(directory):
    """Index top-level timelimit values, rejecting ambiguous problem names."""
    found = {}
    for path in sorted(directory.glob("*/*/info.toml")):
        text = path.read_text(encoding="utf-8")
        lines = text.splitlines()
        top_level = []
        for line in lines:
            if line.lstrip().startswith("["):
                break
            match = LIMIT.fullmatch(line)
            if match:
                top_level.append(float(match.group(1)))
        if len(top_level) != 1 or not math.isfinite(top_level[0]) or top_level[0] <= 0:
            continue
        name = path.parent.name
        value = {"seconds": top_level[0], "path": path.relative_to(directory).as_posix(),
                 "sha256": hashlib.sha256(path.read_bytes()).hexdigest()}
        if name in found:
            raise ValueError(f"duplicate Library Checker problem name: {name}")
        found[name] = value
    if not found:
        raise ValueError(f"no problem limits found under {directory}")
    return found


def valid_seconds(value):
    return (not isinstance(value, bool) and isinstance(value, (int, float))
            and math.isfinite(value) and value >= 0)


def audit(report, repo, limits):
    rows = report.get("results", [])
    if report.get("schema") != 1 or not isinstance(rows, list):
        raise ValueError("unsupported verification report")
    grouped = {}
    for row in rows:
        grouped.setdefault(row["path"], []).append(row)
    inventory = {p.relative_to(repo).as_posix() for p in (repo / "verify").rglob("*.test.cpp")}
    result = []
    for path in sorted(inventory | set(grouped)):
        source = repo / path
        contents = source.read_bytes() if source.is_file() else None
        match = PROBLEM.search(contents.decode("utf-8")) if contents is not None else None
        current_hash = hashlib.sha256(contents).hexdigest() if contents is not None else None
        entries = grouped.get(path, [{"status": "missing", "environment_index": None}])
        for row in entries:
            # A changed or unverifiable driver could point at another problem.
            same_source = current_hash is not None and row.get("verifier_sha256") == current_hash
            name = match.group(1) if match and same_source else None
            limit = limits.get(name)
            item = {"path": path, "environment_index": row.get("environment_index"),
                    "verification_status": row.get("status", "missing"), "problem": name,
                    "limit": limit, "worst_case_seconds": None, "ratio": None}
            if row.get("status") != "passed" or not valid_seconds(row.get("worst_case_seconds")):
                item["assessment"] = "unmeasured"
            elif not same_source:
                item["assessment"] = "source_changed_or_unknown"
                item["worst_case_seconds"] = row["worst_case_seconds"]
            elif limit is None:
                item["assessment"] = "limit_unknown"
                item["worst_case_seconds"] = row["worst_case_seconds"]
            else:
                item["worst_case_seconds"] = row["worst_case_seconds"]
                item["ratio"] = row["worst_case_seconds"] / limit["seconds"]
                item["assessment"] = ("at_or_above_limit" if item["ratio"] >= 1 else
                                      "review_headroom" if item["ratio"] >= 0.5 else "below_half")
            result.append(item)
    return result


def git_revision(directory):
    try:
        top = subprocess.check_output(["git", "-C", str(directory), "rev-parse", "--show-toplevel"],
                                      text=True, stderr=subprocess.DEVNULL).strip()
        if Path(top).resolve() != directory.resolve():
            return None, None
        head = subprocess.check_output(["git", "-C", str(directory), "rev-parse", "HEAD"],
                                       text=True, stderr=subprocess.DEVNULL).strip()
        dirty = bool(subprocess.check_output(["git", "-C", str(directory), "status", "--porcelain"],
                                             text=True, stderr=subprocess.DEVNULL).strip())
        return head, dirty
    except (OSError, subprocess.CalledProcessError):
        return None, None


def render(report, rows, problems, problems_revision, problems_dirty, head, dirty):
    lines = ["# Local verification time-limit screening", "",
             f"Recorded revision: `{report.get('revision', 'unknown')}`; current HEAD: `{head or 'unknown'}`; worktree dirty: `{dirty}`",
             f"Recorded at: `{report.get('generated_at', 'unknown')}`; correctness timeout: `{report.get('environment', {}).get('tle_seconds', 'unknown')}` seconds",
             f"Problem repository: `{problems}`; revision: `{problems_revision or 'unknown'}`; worktree dirty: `{problems_dirty}`", "",
             "The correctness timeout is not the judge time limit. Ratios use each local run's median worst-case time, including local I/O and process overhead. Machine, compiler, data and revision may differ from the judge. No row establishes an AC or TLE verdict.",
             "At or above 100% and at or above 50% are screening priorities, not portable pass/fail thresholds. Unknown limits and unmeasured rows are never treated as zero. A changed driver is not mapped to a problem; a changed library or dirty worktree still requires a fresh measurement.", "",
             "| Verify | Environment | Status | Problem | Local worst-case median (s) | Judge limit (s) | Ratio | Screening |",
             "| --- | ---: | --- | --- | ---: | ---: | ---: | --- |"]
    order = {"at_or_above_limit": 0, "review_headroom": 1, "unmeasured": 2,
             "source_changed_or_unknown": 3, "limit_unknown": 4, "below_half": 5}
    for row in sorted(rows, key=lambda r: (order[r["assessment"]], -(r["ratio"] or 0), r["path"])):
        value = row["worst_case_seconds"]
        limit = row["limit"]
        ratio = f"{row['ratio']:.2f}" if row["ratio"] is not None else "—"
        lines.append(f"| {row['path']} | {row['environment_index']} | {row['verification_status']} | "
                     f"{row['problem'] or 'unknown'} | {value if value is not None else '—'} | "
                     f"{limit['seconds'] if limit else '—'} | "
                     f"{ratio} | {row['assessment']} |")
    lines += ["", "Limits come from the problem repository's `info.toml` files. Preserve the JSON report for source paths and hashes.", ""]
    return "\n".join(lines)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--report", type=Path, default=ROOT / ".verification/current.json")
    parser.add_argument("--problem-repo", type=Path, required=True,
                        help="local checkout of yosupo06/library-checker-problems")
    parser.add_argument("--output", type=Path, required=True, help="output directory for screening.json and screening.md")
    args = parser.parse_args(argv)
    report = json.loads(args.report.read_text(encoding="utf-8"))
    rows = audit(report, ROOT, official_limits(args.problem_repo))
    problems_revision, problems_dirty = git_revision(args.problem_repo)
    head, dirty = git_revision(ROOT)
    output = {"schema": 1, "recorded_revision": report.get("revision"),
              "recorded_at": report.get("generated_at"), "recorded_environment": report.get("environment"),
              "current_head": head, "current_worktree_dirty": dirty,
              "problem_repo": str(args.problem_repo.resolve()), "problem_revision": problems_revision,
              "problem_worktree_dirty": problems_dirty, "results": rows}
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "screening.json").write_text(json.dumps(output, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    markdown = render(report, rows, args.problem_repo, problems_revision, problems_dirty, head, dirty)
    (args.output / "screening.md").write_text(markdown, encoding="utf-8")
    print(markdown)
    return 0


if __name__ == "__main__":
    sys.exit(main())
