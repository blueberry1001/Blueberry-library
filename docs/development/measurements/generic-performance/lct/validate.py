#!/usr/bin/env python3
"""Compile the strengthened LCT random test and compare official exact outputs."""
import argparse
import hashlib
import itertools
import json
from pathlib import Path
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[4]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--output", type=Path, required=True)
    p.add_argument("--problem-root", type=Path, required=True)
    args = p.parse_args()
    args.output.mkdir(parents=True, exist_ok=False)
    sources = [ROOT / "blueberry/graph/link-cut-tree.hpp", ROOT / "tests/random/link-cut-tree.cpp",
               ROOT / "verify/graph/link-cut-tree.test.cpp", ROOT / "verify/graph/link-cut-tree-composite.test.cpp",
               ROOT / "blueberry/utility/fast-io.hpp"]
    hashes = {str(f.relative_to(ROOT)): sha(f) for f in sources if f.exists()}
    results = []
    cases = []
    with tempfile.TemporaryDirectory(prefix="blueberry-lct-validation-") as td:
        temp = Path(td)
        with (args.output / "commands.log").open("w") as log:
            def command(cmd, **kwargs):
                log.write(json.dumps([str(x) for x in cmd]) + "\n"); log.flush()
                return subprocess.run(cmd, stderr=log, check=True, **kwargs)
            for compiler, standard in itertools.product(["g++", "clang++"], ["gnu++20", "gnu++23"]):
                binary = temp / (compiler + standard)
                command([compiler, "-std=" + standard, "-O2", "-Wall", "-Wextra", "-I", ROOT,
                         ROOT / "tests/random/link-cut-tree.cpp", "-o", binary], stdout=log)
                for seed in [1, 2, 3, 4, 5, 20260919]:
                    command([binary, str(seed)], stdout=log, timeout=60)
                results.append({"compiler": compiler, "standard": standard, "seeds": [1, 2, 3, 4, 5, 20260919], "passed": True})
            binary = temp / "sanitizer"
            command(["clang++", "-std=gnu++20", "-O1", "-g", "-fsanitize=address,undefined",
                     "-fno-omit-frame-pointer", "-I", ROOT, ROOT / "tests/random/link-cut-tree.cpp", "-o", binary], stdout=log)
            command([binary, "20260919"], stdout=log, timeout=120)
            results.append({"compiler": "clang++", "standard": "gnu++20", "sanitizers": ["address", "undefined"], "seeds": [20260919], "passed": True})
            for driver, problem in [("link-cut-tree", "dynamic_tree_vertex_add_path_sum"),
                                    ("link-cut-tree-composite", "dynamic_tree_vertex_set_path_composite")]:
                binary = temp / driver
                command(["g++", "-std=gnu++20", "-O2", "-I", ROOT, "-I", ROOT / ".deps/ac-library",
                         ROOT / ("verify/graph/" + driver + ".test.cpp"), "-o", binary], stdout=log)
                problem_dir = args.problem_root / "tree" / problem
                inputs = sorted((problem_dir / "in").glob("*.in"))
                if not inputs:
                    raise RuntimeError(f"no generated cases: {problem_dir}")
                for inp in inputs:
                    out = problem_dir / "out" / (inp.stem + ".out")
                    with inp.open("rb") as stdin:
                        run = command([binary], stdin=stdin, stdout=subprocess.PIPE, timeout=60)
                    if run.stdout.split() != out.read_bytes().split():
                        raise RuntimeError(f"wrong answer: {problem}/{inp.name}")
                    cases.append({"problem": problem, "case": inp.stem, "input_sha256": sha(inp), "output_sha256": sha(out), "passed": True})
    if hashes != {f: sha(ROOT / f) for f in hashes}:
        raise RuntimeError("validation source changed")
    (args.output / "results.json").write_text(json.dumps({"sources": hashes, "random": results, "official": cases, "passed": True}, indent=2) + "\n")
    print(f"PASS: 24 random matrix runs, 1 ASan/UBSan run, {len(cases)} official cases")


if __name__ == "__main__":
    main()
