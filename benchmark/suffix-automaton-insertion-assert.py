#!/usr/bin/env python3
"""Assertion-enabled SAM insertion trial; keep release measurements separate."""

import importlib.util
import pathlib
import sys


ROOT = pathlib.Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("sam_insertion_assert", ROOT / "benchmark/suffix-automaton-insertion.py")
insertion = importlib.util.module_from_spec(spec)
spec.loader.exec_module(insertion)
insertion.profile.BUILD = ROOT / ".build/suffix-automaton-insertion-assert"
insertion.profile.OUT = ROOT / "benchmark/results/suffix-automaton-insertion-assert"

if __name__ == "__main__":
    sys.argv.append("--assertions")
    insertion.profile.main()
