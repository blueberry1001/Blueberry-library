"""Run existing generated Library Checker cases through their official checker."""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
import time
ROOT = Path(__file__).resolve().parents[1]
CACHE = Path.home()/".cache/online-judge-tools/library-checker-problems"
OUT = ROOT/"benchmark/results/contest-template-import"
BUILD = ROOT/".build/contest-import"
rows = []
parser = argparse.ArgumentParser()
parser.add_argument("--problem", choices=["binomial_coefficient_prime_mod", "many_aplusb_128bit"])
args = parser.parse_args()
report_name = "official-128bit.json" if args.problem == "many_aplusb_128bit" else "official.json"
revision = subprocess.check_output(["git", "-C", str(CACHE), "rev-parse", "HEAD"], text=True).strip()
for compiler in ["g++", "clang++"]:
    for standard in ["gnu++20", "gnu++23"]:
        for problem_name, prefix in [("binomial_coefficient_prime_mod", "verify"), ("many_aplusb_128bit", "stream-verify")]:
          if args.problem and args.problem != problem_name: continue
          problem = next(CACHE.glob(f"*/{problem_name}"))
          executable = BUILD/f"{prefix}-{compiler}-{standard}"
          for source in sorted((problem/"in").glob("*.in")):
            actual = BUILD/"actual.out"
            start = time.monotonic()
            with source.open("rb") as input_file, actual.open("wb") as output_file:
                result = subprocess.run([str(executable)], stdin=input_file, stdout=output_file, stderr=subprocess.PIPE, timeout=15)
            elapsed = time.monotonic()-start
            answer = problem/"out"/source.with_suffix(".out").name
            checker = subprocess.run([str(problem/"checker"), str(source), str(actual), str(answer)], capture_output=True, text=True)
            row = dict(problem=problem_name, compiler=compiler, standard=standard, case=source.name, elapsed_seconds=elapsed,
                       input_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),
                       solution_status=result.returncode, solution_stderr=result.stderr.decode(),
                       checker_status=checker.returncode, checker_stdout=checker.stdout, checker_stderr=checker.stderr)
            rows.append(row)
            (OUT/report_name).write_text(json.dumps(dict(upstream_revision=revision, results=rows), indent=2)+"\n")
            if result.returncode or checker.returncode: raise RuntimeError(row)
print(f"Passed {len(rows)} official case/compiler/standard checks")
