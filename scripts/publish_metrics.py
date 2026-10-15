#!/usr/bin/env python3
"""Publish current measurements separately from source-validated AC provenance."""
import json
from pathlib import Path
import subprocess

try:
    from .verify_with_metrics import validate_plan_against_repository, validate_publication_report
except ImportError:
    from verify_with_metrics import validate_plan_against_repository, validate_publication_report


def publication_state(basedir, revision):
    try:
        from .verification_state import load_state, valid_successes
    except ImportError:
        from verification_state import load_state, valid_successes
    state = load_state(basedir / ".verification/state.json")
    return {"schema": 1, "revision": revision, "records": valid_successes(state, basedir),
            "failures": state.get("failures", {})}


def publish_metrics(basedir=None):
    basedir = Path.cwd() if basedir is None else Path(basedir).resolve()
    source = basedir / ".verification/current.json"
    revision = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=basedir, text=True).strip()
    report = None
    if source.exists():
        report = json.loads(source.read_text(encoding="utf-8"))
        expected = {p.relative_to(basedir).as_posix() for p in (basedir / "verify").rglob("*.test.cpp")}
        validate_publication_report(report, expected, revision)
        if report.get("selection") is not None:
            validate_plan_against_repository(report["selection"], basedir)
    state = publication_state(basedir, revision)
    if report is not None:
        missing = expected - set(state["records"])
        if missing:
            raise ValueError("publication requires source-valid AC evidence for every verifier; missing: "
                             + ", ".join(sorted(missing)))
    output = basedir / ".verify-helper/markdown"
    for name, asset, payload in (
        ("verification_metrics.json", "verification-metrics.json", report),
        ("verification_state.json", "verification-state.json", state),
    ):
        for destination in (output / "_data" / name, output / "assets" / asset):
            if payload is None:
                # A previous generated report must never masquerade as this run.
                destination.unlink(missing_ok=True)
                continue
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    if report is None:
        print("No current measurement; retained source-valid AC provenance is published separately.")
    return state


if __name__ == "__main__":
    publish_metrics()
