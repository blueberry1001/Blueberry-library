#!/usr/bin/env python3
"""Run Python/UI unit tests; optionally omit the known C++ integration tests."""
import argparse
from pathlib import Path
import sys
import unittest

CPP_MODULES = {"test_tree_random", "test_data_structures"}
CPP_TESTS = {"test_benchmark_runner.BenchmarkRunnerTests.test_static_rmq_implementations_agree"}


def cases(suite):
    for item in suite:
        if isinstance(item, unittest.TestSuite):
            yield from cases(item)
        else:
            yield item


def without_cpp(suite):
    kept = unittest.TestSuite()
    for case in cases(suite):
        if case.id().split(".")[0] in CPP_MODULES or case.id() in CPP_TESTS:
            print(f"NOT SELECTED (no C++ changes): {case.id()}", flush=True)
        else:
            kept.addTest(case)
    return kept


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--without-cpp", action="store_true")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    sys.path.insert(0, str(root))
    suite = unittest.defaultTestLoader.discover(str(root / "tests"))
    if args.without_cpp:
        suite = without_cpp(suite)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    raise SystemExit(not result.wasSuccessful())


if __name__ == "__main__":
    main()
