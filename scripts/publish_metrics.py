#!/usr/bin/env python3
"""Copy generated measurements into Jekyll's data and public assets."""
import json
from pathlib import Path
import shutil
import subprocess

from verify_with_metrics import validate_plan_against_repository, validate_publication_report

source = Path(".verification/current.json")
if source.exists():
    report = json.loads(source.read_text())
    if report.get("schema") != 1:
        raise SystemExit("Unsupported verification metrics schema")
    expected = {p.as_posix() for p in Path("verify").rglob("*.test.cpp")}
    revision = subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip()
    validate_publication_report(report, expected, revision)
    if report.get("selection") is not None:
        validate_plan_against_repository(report["selection"])
    for destination in (
        Path(".verify-helper/markdown/_data/verification_metrics.json"),
        Path(".verify-helper/markdown/assets/verification-metrics.json"),
    ):
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, destination)
else:
    print("No measurement yet; documentation will display the empty state.")
