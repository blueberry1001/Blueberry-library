#!/usr/bin/env python3
"""Copy generated measurements into Jekyll's data and public assets."""
import json
from pathlib import Path
import shutil

source = Path(".verification/current.json")
if source.exists():
    report = json.loads(source.read_text())
    if report.get("schema") != 1:
        raise SystemExit("Unsupported verification metrics schema")
    for destination in (
        Path(".verify-helper/markdown/_data/verification_metrics.json"),
        Path(".verify-helper/markdown/assets/verification-metrics.json"),
    ):
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, destination)
else:
    print("No measurement yet; documentation will display the empty state.")
