#!/usr/bin/env python3
"""Focused LCA correctness checks. Run after performance measurements."""
import json
from pathlib import Path
import shlex
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[4]


def main():
    results = []
    with (HERE / "validation.log").open("w") as log, tempfile.TemporaryDirectory(prefix="lca-validation-") as td:
        def execute(command, *, source=None):
            log.write("$ " + shlex.join(map(str, command)) + "\n"); log.flush()
            proc = subprocess.run(command, input=source, text=True, capture_output=True, cwd=ROOT, timeout=180)
            log.write(proc.stdout + proc.stderr); log.flush()
            results.append({"command": list(map(str, command)), "exit_code": proc.returncode})
            (HERE / "validation.json").write_text(json.dumps(results, indent=2) + "\n")
            if proc.returncode:
                raise RuntimeError("check failed: " + shlex.join(map(str, command)))

        for compiler in ["g++", "clang++"]:
            for standard in ["gnu++20", "gnu++23"]:
                flags = [compiler, "-std=" + standard, "-O2", "-Wall", "-Wextra", "-Wshadow", "-Werror", "-I", str(ROOT)]
                for filename in ["tests/random/lca-boundary.cpp", "tests/tree_random.cpp"]:
                    binary = str(Path(td) / (compiler + standard + Path(filename).stem))
                    execute([*flags, filename, "-o", binary])
                    if "lca-boundary" in filename:
                        for seed in range(1, 21): execute([binary, str(seed)])
                    else: execute([binary])
                for filename in ["verify/graph/lowest-common-ancestor.test.cpp", "verify/graph/lowest-common-ancestor-rmq.test.cpp"]:
                    execute([*flags, filename, "-o", str(Path(td) / "verify")])
                include_source = '#include "blueberry/graph/lowest-common-ancestor.hpp"\n' * 2 + 'int main() {}\n'
                execute([*flags, "-DNDEBUG", "-x", "c++", "-", "-o", str(Path(td) / "header")], source=include_source)
        sanitized = str(Path(td) / "sanitized")
        execute(["g++", "-std=gnu++20", "-O1", "-g", "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
                 "-I", str(ROOT), "tests/random/lca-boundary.cpp", "-o", sanitized])
        execute([sanitized, "20261017"])
    print("focused checks passed:", len(results), flush=True)


if __name__ == "__main__":
    main()
