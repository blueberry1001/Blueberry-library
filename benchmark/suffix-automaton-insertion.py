#!/usr/bin/env python3
"""Isolated try_emplace experiment; preserve the earlier clone-move experiment."""

import importlib.util
import pathlib


ROOT = pathlib.Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("tree_sam_profile", ROOT / "benchmark/static-top-tree-sam.py")
profile = importlib.util.module_from_spec(spec)
spec.loader.exec_module(profile)
profile.BUILD = ROOT / ".build/suffix-automaton-insertion"
profile.OUT = ROOT / "benchmark/results/suffix-automaton-insertion"
all_cases = profile.cases
profile.cases = lambda small=False: [case for case in all_cases(small) if case[0] == "sam"]


def candidate_source(path, source):
    if not path.endswith("suffix-automaton.hpp"):
        return source
    before = """    while (p != -1 && !states_[p].next.contains(symbol)) {
      states_[p].next.emplace(symbol, current);
      p = states_[p].link;
    }
"""
    after = """    while (p != -1) {
      if (!states_[p].next.try_emplace(symbol, current).second) break;
      p = states_[p].link;
    }
"""
    return profile.replace_once(source, before, after)


profile.candidate_source = candidate_source

if __name__ == "__main__":
    profile.main()
