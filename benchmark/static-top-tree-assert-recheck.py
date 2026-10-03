#!/usr/bin/env python3
"""Nine-sample focused recheck with the exact already-prepared assert binaries."""

import argparse
import importlib.util
import pathlib


ROOT = pathlib.Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("stt_assert_recheck", ROOT / "benchmark/static-top-tree-assert.py")
experiment = importlib.util.module_from_spec(spec)
spec.loader.exec_module(experiment)
profile = experiment.profile
prepared = profile.checked_prepared()
profile.OUT = ROOT / "benchmark/results/static-top-tree-assert-recheck"
profile.OUT.mkdir(parents=True, exist_ok=True)
if (profile.OUT / "prepared.json").exists():
    raise RuntimeError("Focused recheck exists; preserve it before another run")
prepared["recheck"] = "Reuse exact original assert binaries; path/u64, balanced/u64, broom/mod998; nine samples plus warmup."
profile.write("prepared.json", prepared)
profile.cases = lambda small=False: [
    ["tree", "path", "u64", 100000, 200000],
    ["tree", "balanced", "u64", 100000, 200000],
    ["tree", "broom", "mod998", 100000, 200000],
]

if __name__ == "__main__":
    profile.run(argparse.Namespace(repeats=9))
