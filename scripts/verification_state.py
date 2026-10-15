#!/usr/bin/env python3
"""Content-addressed AC evidence, separate from measurements of the current run.

State is an evidence ledger, not a timestamp cache. Reuse requires identical
tracked transitive source blobs, verifier configuration, requirements and ACL pin.
Failures retain tombstones so an older bootstrap cannot resurrect a rejected AC.
"""
from __future__ import annotations

import argparse
import ast
from copy import deepcopy
from datetime import datetime
from functools import lru_cache
import hashlib
import json
import math
from pathlib import Path, PurePosixPath
import posixpath
import re
import subprocess


HEX = re.compile(r"[0-9a-f]{64}\Z")
REVISION = re.compile(r"[0-9a-f]{40,64}\Z")
MEASUREMENT_KEYS = (
    "environment_index", "compiler", "flags", "verifier_sha256", "dataset_sha256",
    "repeat_count", "case_count", "compile_seconds", "solution_seconds",
    "worst_case_seconds", "min_solution_seconds", "max_solution_seconds",
)
EXECUTION_NODES = {"read_samples", "digest_files", "Recorder", "run_with_helper"}


def execution_contract(source):
    """Fingerprint checker/compile/execute semantics, excluding report plumbing."""
    if source is None:
        return state_digest({"execution_recorder": "absent"})
    tree = ast.parse(source)
    nodes = [node for node in tree.body if isinstance(node, (ast.FunctionDef, ast.ClassDef))
             and node.name in EXECUTION_NODES]
    if {node.name for node in nodes} != EXECUTION_NODES or len(nodes) != len(EXECUTION_NODES):
        raise ValueError("verification execution core is incomplete")
    nodes.sort(key=lambda node: node.name)

    def dump(node):
        try:
            # Python 3.13 otherwise omits empty fields that 3.12 included. The
            # bootstrap and CI must agree even when their Python minors differ.
            return ast.dump(node, include_attributes=False, show_empty=True)
        except TypeError:  # Python 3.12 has no show_empty keyword.
            return ast.dump(node, include_attributes=False)

    return state_digest([dump(node) for node in nodes])


def empty_state():
    return {"schema": 1, "records": {}, "failures": {}}


def state_digest(state):
    return hashlib.sha256(json.dumps(state, sort_keys=True, separators=(",", ":"),
                                     ensure_ascii=False, allow_nan=False).encode()).hexdigest()


def _timestamp(value):
    if not isinstance(value, str):
        raise ValueError("missing evidence timestamp")
    result = datetime.fromisoformat(value.replace("Z", "+00:00"))
    if result.tzinfo is None:
        raise ValueError("evidence timestamp must include timezone")
    return result


def _verifier(path):
    return (isinstance(path, str) and path.startswith("verify/") and path.endswith(".test.cpp")
            and "\\" not in path and ".." not in PurePosixPath(path).parts
            and PurePosixPath(path).as_posix() == path)


def _valid_measurement(row):
    if not isinstance(row, dict):
        return False
    if not all(isinstance(row.get(key), str) and HEX.fullmatch(row[key])
               for key in ("verifier_sha256", "dataset_sha256")):
        return False
    if (type(row.get("repeat_count")) is not int or row["repeat_count"] < 1 or
            type(row.get("case_count")) is not int or row["case_count"] < 1 or
            type(row.get("environment_index")) is not int or row["environment_index"] < 0 or
            not isinstance(row.get("compiler"), str) or not row["compiler"] or
            not isinstance(row.get("flags"), list) or
            any(not isinstance(flag, str) for flag in row["flags"])):
        return False
    return all(type(row.get(key)) in (int, float) and math.isfinite(row[key]) and row[key] >= 0
               for key in ("compile_seconds", "solution_seconds", "worst_case_seconds",
                           "min_solution_seconds", "max_solution_seconds"))


def validated_state(state):
    """Reject the entire corrupt ledger, rather than trust its remaining AC rows."""
    try:
        if (not isinstance(state, dict) or type(state.get("schema")) is not int or state["schema"] != 1 or
                not isinstance(state.get("records"), dict) or
                not isinstance(state.get("failures", {}), dict)):
            return empty_state()
        for section in ("records", "failures"):
            for path, row in state.get(section, {}).items():
                if (not _verifier(path) or not isinstance(row, dict) or
                        not isinstance(row.get("fingerprint"), str) or not HEX.fullmatch(row["fingerprint"]) or
                        not isinstance(row.get("verified_revision"), str) or
                        not REVISION.fullmatch(row["verified_revision"]) or
                        not isinstance(row.get("run_url"), str) or
                        (row["run_url"] and not row["run_url"].startswith("https://"))):
                    return empty_state()
                _timestamp(row.get("verified_at"))
                if section == "records":
                    measurement = row.get("measurement")
                    if (not _valid_measurement(measurement) or
                            not isinstance(measurement.get("row_sha256"), str) or
                            not HEX.fullmatch(measurement["row_sha256"]) or
                            type(measurement.get("environment_count")) is not int or
                            measurement["environment_count"] < 1):
                        return empty_state()
        result = deepcopy(state)
        result.setdefault("failures", {})
        return result
    except (ValueError, TypeError, OverflowError):
        return empty_state()


def load_state(path):
    try:
        return validated_state(json.loads(Path(path).read_text(encoding="utf-8")))
    except (OSError, ValueError):
        return empty_state()


def merge_states(seed, latest, *additional):
    """Merge provenance chronologically; a failure wins a same-time tie."""
    result = empty_state()
    for source in map(validated_state, (seed, latest, *additional)):
        for section in ("records", "failures"):
            for path, row in source[section].items():
                opposite = "failures" if section == "records" else "records"
                old = result[section].get(path) or result[opposite].get(path)
                if old is not None:
                    old_time, new_time = _timestamp(old["verified_at"]), _timestamp(row["verified_at"])
                    if new_time < old_time or (new_time == old_time and opposite == "failures" and
                                               path in result["failures"]):
                        continue
                result[opposite].pop(path, None)
                result[section][path] = deepcopy(row)
    return result


@lru_cache(maxsize=8)
def _snapshot_fingerprints(repo, sha):
    # Lazy imports let plan_verification call this module without a circular import.
    try:
        from .plan_verification import Snapshot, includes, CPP_SUFFIXES
    except ImportError:
        from plan_verification import Snapshot, includes, CPP_SUFFIXES
    snapshot = Snapshot(Path(repo), sha)
    # Batch-read source blobs: process startup otherwise dominates small headers.
    names = [p for p, (mode, kind, _) in snapshot.files.items()
             if kind == "blob" and mode != "120000" and PurePosixPath(p).suffix in CPP_SUFFIXES]
    oids = [snapshot.files[p][2] for p in names]
    output = subprocess.run(["git", "-C", repo, "cat-file", "--batch"],
                            input=("\n".join(oids) + "\n").encode(), check=True,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE).stdout
    offset = 0
    for name in names:
        end = output.index(b"\n", offset)
        size = int(output[offset:end].split()[2])
        try:
            snapshot.cache[name] = output[end + 1:end + 1 + size].decode("utf-8")
        except UnicodeError:
            pass  # Only consumers of an unreadable source lose their evidence.
        offset = end + 1 + size + 1
    dependencies = {}
    edges = {}

    def children(path):
        if path not in edges:
            result = []
            for delimiter, name in includes(snapshot.read(path)):
                candidates = ([posixpath.normpath(posixpath.join(posixpath.dirname(path), name))]
                              if delimiter == '"' else []) + [posixpath.normpath(name)]
                local = next((p for p in candidates if p in snapshot.files), None)
                if local:
                    result.append(local)
                elif ((delimiter == '"' and not name.startswith("atcoder/")) or
                      name.startswith(("blueberry/", "verify/"))):
                    raise ValueError(f"unresolved repository include: {path}: {name}")
            edges[path] = result
        return edges[path]

    for verifier in snapshot.verifiers():
        seen, pending = set(), [verifier]
        try:
            while pending:
                path = pending.pop()
                if path not in seen:
                    seen.add(path)
                    pending.extend(children(path))
            dependencies[verifier] = seen
        except (ValueError, KeyError, UnicodeError):
            continue  # Missing dependency evidence never authorizes this verifier.
    workflow = snapshot.read(".github/workflows/verify.yml")
    acl_pins = set(re.findall(r"repository:\s*atcoder/ac-library\s+ref:\s*([0-9a-f]{40})\b", workflow))
    if len(acl_pins) != 1:
        raise ValueError("an explicit, consistent ACL checkout pin is required")
    recorder = "scripts/verify_with_metrics.py"
    common = {"acl_commit": next(iter(acl_pins)),
              "execution_contract": execution_contract(snapshot.read(recorder) if recorder in snapshot.files else None)}
    for path in ("requirements-dev.txt", ".verify-helper/config.toml"):
        snapshot.read(path)  # Reject symlinks or absent configuration.
        common[path] = snapshot.files[path][2]
    fingerprints = {}
    verifier_digests = {}
    for path, files in dependencies.items():
        blobs = {name: snapshot.files[name][2] for name in sorted(files)}
        fingerprints[path] = state_digest({"version": 1, "blobs": blobs, "configuration": common})
        verifier_digests[path] = hashlib.sha256(snapshot.read(path).encode()).hexdigest()
    return fingerprints, verifier_digests, dependencies, common["acl_commit"], common["execution_contract"]


def fingerprints(repo=Path.cwd(), ref="HEAD"):
    """Unknown/missing dependency graphs cannot authorize any saved AC."""
    try:
        try:
            from .plan_verification import revision
        except ImportError:
            from plan_verification import revision
        sha = revision(Path(repo), ref)
        return dict(_snapshot_fingerprints(str(Path(repo).resolve()), sha)[0])
    except (subprocess.CalledProcessError, ValueError, KeyError, UnicodeError, SyntaxError):
        return {}


def valid_successes(state, repo=Path.cwd(), ref="HEAD"):
    state = merge_states(empty_state(), state)
    current = fingerprints(repo, ref)
    invalid = set()
    try:
        try:
            from .plan_verification import git, revision
        except ImportError:
            from plan_verification import git, revision
        sha = revision(Path(repo), ref)
        if sha == revision(Path(repo), "HEAD") and current:
            _, _, dependencies, acl_pin, contract = _snapshot_fingerprints(str(Path(repo).resolve()), sha)
            recorder = "scripts/verify_with_metrics.py"
            relevant = {"requirements-dev.txt", ".verify-helper/config.toml", ".github/workflows/verify.yml", recorder}
            relevant.update(name for files in dependencies.values() for name in files)
            dirty = set(git(Path(repo), "diff", "--name-only", "-z", "HEAD", "--",
                            *sorted(relevant)).decode().split("\0"))
            if dirty.intersection({"requirements-dev.txt", ".verify-helper/config.toml"}):
                return {}
            if ".github/workflows/verify.yml" in dirty:
                workflow = (Path(repo) / ".github/workflows/verify.yml").read_text(encoding="utf-8")
                pins = set(re.findall(r"repository:\s*atcoder/ac-library\s+ref:\s*([0-9a-f]{40})\b", workflow))
                if pins != {acl_pin}:
                    return {}
            if recorder in dirty:
                source = Path(repo) / recorder
                if execution_contract(source.read_text(encoding="utf-8") if source.exists() else None) != contract:
                    return {}
            invalid = {path for path, files in dependencies.items() if dirty.intersection(files)}
    except (subprocess.CalledProcessError, OSError, ValueError, KeyError, UnicodeError, SyntaxError):
        return {}
    return {path: deepcopy(row) for path, row in state["records"].items()
            if path not in invalid and current.get(path) == row["fingerprint"]}


def update_state(state, report, repo=Path.cwd(), run_url="", ref="HEAD"):
    """Add actual report results, retaining their measured revision and timestamp.

    A report from an older commit can bootstrap unchanged contents, never relabel
    old measurements as a new run. Failed/partial runs can invalidate old AC;
    not_selected and not_run rows cannot create evidence. Every environment row
    for a verifier must pass before that verifier receives a new AC record.
    """
    current = fingerprints(repo, ref)
    run_url = run_url or ""
    existing = merge_states(empty_state(), state)
    if (not isinstance(report, dict) or report.get("schema") != 1 or
            not isinstance(report.get("results"), list) or
            any(not isinstance(row, dict) for row in report["results"])):
        raise ValueError("unsupported verification report")
    measured = report.get("revision")
    if not isinstance(measured, str) or not REVISION.fullmatch(measured):
        raise ValueError("missing measured revision")
    _timestamp(report.get("generated_at"))
    verified_at = report.get("completed_at", report["generated_at"])
    _timestamp(verified_at)
    try:
        measured_fingerprints, verifier_digests, _, _, _ = _snapshot_fingerprints(str(Path(repo).resolve()), measured)
    except (subprocess.CalledProcessError, ValueError, KeyError, UnicodeError, SyntaxError) as error:
        raise ValueError("measured source snapshot is unavailable") from error
    updates = empty_state()
    groups = {}
    try:
        from .plan_verification import Snapshot
    except ImportError:
        from plan_verification import Snapshot
    measured_paths = set(Snapshot(Path(repo), measured).verifiers())
    for row in report["results"]:
        path = row.get("path")
        if not _verifier(path) or path not in measured_paths:
            raise ValueError("unknown verification report path")
        groups.setdefault(path, []).append(row)
    for path, rows in groups.items():
        evidence = {"fingerprint": measured_fingerprints.get(path) or
                    state_digest({"unresolved_verifier": path, "revision": measured}), "verified_revision": measured,
                    "verified_at": verified_at, "run_url": run_url}
        if any(row.get("status") == "failed" for row in rows):
            updates["failures"][path] = evidence
        elif all(row.get("status") == "passed" for row in rows):
            if (path not in measured_fingerprints or
                    any(not _valid_measurement(row) or row["verifier_sha256"] != verifier_digests[path] or
                    not isinstance(row.get("runs"), list) or len(row["runs"]) != row["repeat_count"]
                    for row in rows) or
                    len({row["environment_index"] for row in rows}) != len(rows)):
                raise ValueError("AC is missing measured environment evidence")
            chosen = min(rows, key=lambda row: row["environment_index"])
            evidence["measurement"] = {key: deepcopy(chosen[key]) for key in MEASUREMENT_KEYS}
            evidence["measurement"]["row_sha256"] = state_digest(chosen)
            evidence["measurement"]["environment_count"] = len(rows)
            updates["records"][path] = evidence
    if validated_state(updates) != updates:
        raise ValueError("invalid provenance in verification report")
    try:
        from .plan_verification import revision
    except ImportError:
        from plan_verification import revision
    if measured == revision(Path(repo), "HEAD") and updates["records"]:
        # The verifier's own digest does not cover dirty included headers. Never
        # attach a clean Git fingerprint to a binary compiled from dirty sources.
        if set(valid_successes(updates, repo, ref=measured)) != set(updates["records"]):
            raise ValueError("passed report has uncommitted source or configuration dependencies")
    merged = merge_states(existing, updates)
    merged["records"] = {path: row for path, row in merged["records"].items()
                         if current.get(path) == row["fingerprint"]}
    current_paths = set(Snapshot(Path(repo), ref).verifiers())
    merged["failures"] = {path: row for path, row in merged["failures"].items() if path in current_paths}
    return merged


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--report", type=Path, required=True)
    parser.add_argument("--state", type=Path, default=Path(".verification/state.json"))
    parser.add_argument("--output", type=Path)
    parser.add_argument("--run-url", default="")
    args = parser.parse_args()
    report = json.loads(args.report.read_text(encoding="utf-8"))
    state = update_state(load_state(args.state), report, run_url=args.run_url)
    output = args.output or args.state
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(state, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n",
                      encoding="utf-8")
    print(f"Saved {len(state['records'])} reusable AC records and {len(state['failures'])} failures")


if __name__ == "__main__":
    main()
