#!/usr/bin/env python3
"""Assertion-enabled STT singleton trial; other production headers stay unchanged."""

import importlib.util
import pathlib
import sys


ROOT = pathlib.Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("tree_sam_assert", ROOT / "benchmark/static-top-tree-sam.py")
profile = importlib.util.module_from_spec(spec)
spec.loader.exec_module(profile)
profile.BUILD = ROOT / ".build/static-top-tree-assert"
profile.OUT = ROOT / "benchmark/results/static-top-tree-assert"
singleton_candidate = profile.candidate_source
all_cases = profile.cases
profile.cases = lambda small=False: [case for case in all_cases(small) if case[0] == "tree"]


def candidate_source(path, source):
    if path.endswith("static-top-tree.hpp"):
        return singleton_candidate(path, source)
    return source


profile.candidate_source = candidate_source

if __name__ == "__main__":
    sys.argv.append("--assertions")
    profile.main()
