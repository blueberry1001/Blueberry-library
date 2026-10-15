"""Same compiler/input end-to-end comparison; research source stays outside Git."""
from pathlib import Path
import hashlib
import json
import subprocess
import time
ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT/".build/contest-import"
OUT = ROOT/"benchmark/results/contest-template-import"
SOURCE = BUILD/"submission-223248.cpp"
PROBLEM = Path.home()/".cache/online-judge-tools/library-checker-problems/enumerative_combinatorics/binomial_coefficient_prime_mod"
case = PROBLEM/"in/mod998244353_maxi_00.in"
binary = BUILD/"fastest-223248"
flags = ["g++", "-std=gnu++20", "-O2", "-DNDEBUG", "-DONLINE_JUDGE"]
driver = BUILD/"verify-matched-flags"
subprocess.run([*flags, "-I.", "-isystem", "/home/blueberry/lib/acl",
                str(ROOT/"verify/math/binomial-coefficient-prime-mod.test.cpp"), "-o", str(driver)], check=True)
compile_result = subprocess.run([*flags, str(SOURCE), "-o", str(binary)], capture_output=True, text=True)
report = dict(source_url="https://judge.yosupo.jp/submission/223248", source_sha256=hashlib.sha256(SOURCE.read_bytes()).hexdigest(),
              flags=flags, compile_status=compile_result.returncode, compile_stderr=compile_result.stderr,
              case=case.name, input_sha256=hashlib.sha256(case.read_bytes()).hexdigest(), results=[])
(OUT/"fastest-local.json").write_text(json.dumps(report, indent=2)+"\n")
if compile_result.returncode: raise RuntimeError(compile_result.stderr)
for repeat in range(3):
    for name, executable in [("fastest-223248",binary), ("blueberry-driver",driver)]:
        actual = BUILD/f"{name}.out"
        start = time.monotonic()
        with case.open("rb") as input_file, actual.open("wb") as output_file:
            result = subprocess.run(["/usr/bin/time", "-f", "%M", str(executable)], stdin=input_file, stdout=output_file, stderr=subprocess.PIPE, text=True, timeout=15)
        elapsed = time.monotonic()-start
        checker = subprocess.run([str(PROBLEM/"checker"), str(case), str(actual), str(PROBLEM/"out/mod998244353_maxi_00.out")], capture_output=True, text=True)
        report["results"].append(dict(variant=name, repeat=repeat, seconds=elapsed, peak_rss_kib=result.stderr.strip(),
                                      status=result.returncode, checker_status=checker.returncode, checker_stderr=checker.stderr))
        (OUT/"fastest-local.json").write_text(json.dumps(report, indent=2)+"\n")
        if result.returncode or checker.returncode: raise RuntimeError(report["results"][-1])
print("Completed local fastest-vs-driver comparison")
