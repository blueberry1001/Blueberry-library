#!/usr/bin/env python3
"""Plan auditable, fail-safe Library Checker coverage from two Git snapshots.

Only explicitly known documentation changes bypass C++ checks. Include edges from
both snapshots retain consumers of deleted/renamed dependencies. Conditional
includes are deliberately over-approximated; unsupported directives select all.
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path, PurePosixPath
import posixpath
import re
import subprocess
from urllib.request import Request, urlopen


CPP_SUFFIXES = {".hpp", ".h", ".hh", ".hxx", ".cpp", ".cc", ".cxx", ".ipp", ".tpp", ".inc"}
DOC_FILES = {"README.md", "LICENSE", "LICENSE.md", "AGENTS.md", "IMPLEMENTATION_POLICY.md",
             "tests/docs_ui_test.cjs", "tests/test_docs_ui.py", "tests/test_operations.py",
             "tests/test_generated_docs.py", "tests/test_library_checker_coverage.py"}
DOC_SCRIPTS = {"scripts/generate_docs.py", "scripts/check_docs.py", "scripts/check_site.py",
               "scripts/library_checker_coverage.py", "scripts/publish_metrics.py"}


def git(repo: Path, *args: str) -> bytes:
    return subprocess.run(["git", "-C", str(repo), *args], check=True,
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE).stdout


def revision(repo: Path, ref: str) -> str:
    # End-of-options prevents user-supplied revisions becoming Git switches.
    return git(repo, "rev-parse", "--verify", "--end-of-options", ref + "^{commit}").decode().strip()


class Snapshot:
    def __init__(self, repo: Path, sha: str):
        self.repo = repo
        self.files = {}
        for entry in git(repo, "ls-tree", "-r", "-z", "--full-tree", sha).split(b"\0"):
            if entry:
                meta, name = entry.split(b"\t", 1)
                mode, kind, oid = meta.decode().split()
                self.files[name.decode("utf-8", "surrogateescape")] = (mode, kind, oid)
        self.cache = {}

    def read(self, path: str) -> str:
        if path not in self.cache:
            mode, kind, oid = self.files[path]
            if kind != "blob" or mode == "120000":
                raise ValueError(f"unsupported include file (symlink/submodule): {path}")
            self.cache[path] = git(self.repo, "cat-file", "blob", oid).decode("utf-8")
        return self.cache[path]

    def verifiers(self) -> list[str]:
        return sorted(p for p in self.files if p.startswith("verify/") and p.endswith(".test.cpp"))


def changed_files(repo: Path, base: str, head: str) -> list[dict]:
    fields = iter(git(repo, "diff", "--name-status", "-z", "--find-renames", base, head, "--").split(b"\0"))
    result = []
    for field in fields:
        if not field:
            continue
        status = field.decode()
        first = next(fields).decode("utf-8", "surrogateescape")
        if status[0] in "RC":
            result.append({"status": status, "previous_path": first,
                           "path": next(fields).decode("utf-8", "surrogateescape")})
        else:
            result.append({"status": status, "path": first})
    return result


def file_kind(path: str) -> str:
    if path in DOC_FILES or path in DOC_SCRIPTS or path.startswith(("docs/", ".verify-helper/docs/", "templates/")):
        return "docs"
    if path.startswith(("blueberry/", "verify/", "tests/")) and PurePosixPath(path).suffix in CPP_SUFFIXES:
        return "cpp"
    # Toolchains, configs, harnesses, dependencies, and unknown files are full.
    return "full"


def includes(source: str) -> list[tuple[str, str]]:
    source = re.sub(r"\\\r?\n", "", source)
    # Preserve strings (including include names) while removing comments.
    token = r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*[\s\S]*?\*/'
    source = re.sub(token, lambda m: "\n" * m[0].count("\n") if m[0].startswith(("//", "/*")) else m[0], source)
    result = []
    for line in source.splitlines():
        directive = re.match(r"^\s*#\s*(include\w*)\b(.*)$", line)
        if not directive:
            continue
        literal = re.fullmatch(r'\s*([<"])([^>"\n]+)[>"]\s*', directive[2])
        if directive[1] != "include" or not literal:
            raise ValueError("macro or unsupported include directive: " + line.strip())
        result.append((literal[1], literal[2]))
    return result


def dependency_sets(snapshot: Snapshot) -> dict[str, set[str]]:
    edges = {}

    def children(path: str) -> list[str]:
        if path in edges:
            return edges[path]
        result = []
        for delimiter, name in includes(snapshot.read(path)):
            candidates = ([posixpath.normpath(posixpath.join(posixpath.dirname(path), name))]
                          if delimiter == '"' else []) + [posixpath.normpath(name)]
            local = next((p for p in candidates if p in snapshot.files), None)
            if local:
                result.append(local)
            elif delimiter == '"' and not name.startswith("atcoder/"):
                raise ValueError(f"unresolved local include: {path}: {name}")
            elif name.startswith(("blueberry/", "verify/")):
                raise ValueError(f"unresolved repository include: {path}: {name}")
            # Other angle includes and ACL are external, with toolchain/config
            # changes independently forcing a full run.
        edges[path] = result
        return result

    result = {}
    for verifier in snapshot.verifiers():
        seen, pending = set(), [verifier]
        while pending:
            path = pending.pop()
            if path not in seen:
                seen.add(path)
                pending.extend(children(path))
        result[verifier] = seen
    return result


def build_plan(repo: Path, base_sha: str | None, head_sha: str, force_full: bool = False,
               full_reason: str = "explicit full verification") -> dict:
    repo = Path(repo)
    head = revision(repo, head_sha)
    current = Snapshot(repo, head)
    all_verify = current.verifiers()
    plan = {"schema_version": 1, "base_sha": None, "head_sha": head,
            "mode": "affected", "all_verify": all_verify, "selected_verify": [],
            "compile_required": False, "repeats": 1, "changed_files": [], "reasons": [],
            "docs_examples": []}

    def full(reason: str) -> dict:
        plan.update(mode="full", selected_verify=all_verify, compile_required=True, docs_examples=None,
                    reason=reason, reasons=[reason])
        return plan

    if force_full:
        if base_sha:
            try:
                plan["base_sha"] = revision(repo, base_sha)
                plan["changed_files"] = changed_files(repo, plan["base_sha"], head)
            except (subprocess.CalledProcessError, ValueError):
                pass
        return full(full_reason)
    if not base_sha:
        return full("base revision unavailable; fail-safe full verification")
    try:
        base = revision(repo, base_sha)
        plan["base_sha"] = base
        changes = changed_files(repo, base, head)
        plan["changed_files"] = changes
    except (subprocess.CalledProcessError, ValueError) as error:
        return full(f"base/diff unavailable; fail-safe full verification ({type(error).__name__})")
    paths = {c[key] for c in changes for key in ("path", "previous_path") if key in c}
    plan["docs_examples"] = sorted(p for p in paths if p in current.files and (
        p in (".verify-helper/docs/static/guide.md", ".verify-helper/docs/static/migration.md") or
        (p.startswith("docs/") and p.endswith(".md") and
         "blueberry/" + p.removeprefix("docs/").removesuffix(".md") + ".hpp" in current.files)))
    unknown = sorted(p for p in paths if file_kind(p) == "full")
    if unknown:
        return full("harness/toolchain/config or unclassified changes: " + ", ".join(unknown))
    cpp = {p for p in paths if file_kind(p) == "cpp"}
    if not cpp:
        plan.update(reason="documentation/UI-only changes" if paths else "no changed files",
                    reasons=["no C++ or verification-harness changes"])
        return plan
    plan["compile_required"] = True
    plan["docs_examples"] = None
    try:
        previous = Snapshot(repo, base)
        old_graph, new_graph = dependency_sets(previous), dependency_sets(current)
    except (subprocess.CalledProcessError, ValueError, UnicodeError) as error:
        return full(f"dependency analysis incomplete; fail-safe full verification: {error}")
    # Renamed verify drivers are already selected because their new path changed.
    # Other consumers survive a header rename even if only the old graph has it.
    selected = [p for p in all_verify if cpp.intersection(old_graph.get(p, set()) | new_graph[p])]
    plan.update(selected_verify=selected, reason="transitive includes in base and head",
                reasons=[f"{p}: " + ", ".join(sorted(cpp.intersection(old_graph.get(p, set()) | new_graph[p])))
                         for p in selected])
    if not selected:
        plan["reasons"] = ["changed C++ has no surviving verification consumer; compile/random checks still required"]
    return plan


def validate_plan(plan: dict, repo: Path = Path.cwd()) -> None:
    if not isinstance(plan, dict) or plan.get("schema_version") != 1:
        raise ValueError("unsupported CI plan schema")
    if plan.get("mode") not in ("full", "affected") or type(plan.get("compile_required")) is not bool:
        raise ValueError("invalid CI plan mode/compile_required")
    if type(plan.get("repeats")) is not int or plan["repeats"] not in (1, 3):
        raise ValueError("invalid CI plan repeats")
    head = revision(Path(repo), "HEAD")
    if plan.get("head_sha") != head:
        raise ValueError("CI plan does not describe checked-out HEAD")
    expected = build_plan(Path(repo), plan.get("base_sha"), head, force_full=plan["mode"] == "full")
    for key in ("mode", "all_verify", "selected_verify", "compile_required", "docs_examples"):
        if plan.get(key) != expected[key]:
            raise ValueError(f"CI plan {key} differs from recomputed dependency selection")


def event_base(repo: Path, event_name: str, event: dict, head: str) -> str | None:
    try:
        if event_name == "pull_request":
            base = revision(repo, event["pull_request"]["base"]["sha"])
            return git(repo, "merge-base", base, revision(repo, head)).decode().strip()
        if event_name == "push":
            before = event.get("before")
            return before if before and set(before) != {"0"} else None
    except (KeyError, subprocess.CalledProcessError, ValueError):
        pass
    return None


def verified_ancestor(repo: Path, candidate: str, runs: list[dict]) -> tuple[str | None, dict | None]:
    """A canceled/failed prior push cannot become the next run's skip baseline."""
    candidate = revision(repo, candidate)
    for run in runs:
        if (run.get("head_branch") != "main" or run.get("event") not in ("push", "workflow_dispatch") or
                run.get("status") != "completed" or run.get("conclusion") != "success"):
            continue
        sha = run.get("head_sha", "")
        if not isinstance(sha, str) or not re.fullmatch(r"[0-9a-f]{40,64}", sha):
            continue
        try:
            sha = revision(repo, sha)
            git(repo, "merge-base", "--is-ancestor", sha, candidate)
        except subprocess.CalledProcessError:
            continue
        return sha, run
    return None, None


def trusted_base(repo: Path, candidate: str | None) -> tuple[str | None, dict]:
    """Use a completed successful main workflow, or conservatively run everything."""
    token, repository = os.environ.get("GITHUB_TOKEN"), os.environ.get("GITHUB_REPOSITORY", "")
    if not candidate or not token or not re.fullmatch(r"[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+", repository):
        return None, {"reason": "verified baseline unavailable (missing event/API context)"}
    api = os.environ.get("GITHUB_API_URL", "https://api.github.com").rstrip("/")
    request = Request(f"{api}/repos/{repository}/actions/workflows/verify.yml/runs"
                      "?branch=main&status=success&per_page=100",
                      headers={"Authorization": f"Bearer {token}", "Accept": "application/vnd.github+json",
                               "X-GitHub-Api-Version": "2022-11-28"})
    try:
        with urlopen(request, timeout=20) as response:
            payload = json.load(response)
        runs = payload["workflow_runs"]
        if not isinstance(runs, list) or not all(isinstance(run, dict) for run in runs):
            raise ValueError("invalid workflow run inventory")
        base, run = verified_ancestor(repo, candidate, runs)
        if base:
            return base, {"reason": "successful main workflow ancestor", "run_id": run.get("id"),
                          "run_url": run.get("html_url")}
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        return None, {"reason": f"verified baseline API unavailable ({type(error).__name__}); full verification"}
    return None, {"reason": "no verified ancestor among recent main workflows; full verification"}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path.cwd())
    parser.add_argument("--base")
    parser.add_argument("--head", default="HEAD")
    parser.add_argument("--event-name", default=os.environ.get("GITHUB_EVENT_NAME", ""))
    parser.add_argument("--event-path", type=Path, default=os.environ.get("GITHUB_EVENT_PATH"))
    parser.add_argument("--full", action="store_true")
    parser.add_argument("--output", type=Path, default=Path(".verification/ci-plan.json"))
    args = parser.parse_args()
    event = {}
    if args.event_path:
        try:
            event = json.loads(args.event_path.read_text())
        except (OSError, ValueError):
            pass  # Missing event context cannot lead to skipped verification.
    full = args.full or args.event_name == "workflow_dispatch"
    candidate = args.base or event_base(args.repo, args.event_name, event, args.head)
    if args.base or full:
        base, evidence = candidate, {"reason": "explicit base/full run"}
    else:
        base, evidence = trusted_base(args.repo, candidate)
    plan = build_plan(args.repo, base, args.head, force_full=full)
    plan["event_base_sha"] = candidate
    plan["baseline_evidence"] = evidence
    plan["repeats"] = 3 if full else 1
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(plan, ensure_ascii=False, indent=2) + "\n")
    # null means the normal complete documentation example suite is required.
    args.output.with_name("ci-doc-examples.json").write_text(json.dumps(plan["docs_examples"]) + "\n")
    print(json.dumps(plan, ensure_ascii=False, indent=2))
    if os.environ.get("GITHUB_OUTPUT"):
        with open(os.environ["GITHUB_OUTPUT"], "a") as stream:
            stream.write(f"compile_required={str(plan['compile_required']).lower()}\n")
            stream.write(f"mode={plan['mode']}\nrepeats={plan['repeats']}\n")


if __name__ == "__main__":
    main()
