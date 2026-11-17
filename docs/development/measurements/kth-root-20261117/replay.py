import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

source = Path(__file__).resolve().parent
root = source.parents[3]
parser = argparse.ArgumentParser()
parser.add_argument("mode", choices=["candidates", "final", "validate", "official"])
args = parser.parse_args()
build = root / ".build"
build.mkdir(exist_ok=True)
work = Path(tempfile.mkdtemp(prefix="kth-root-replay-", dir=build))
for name in ["baseline.hpp", "candidate.hpp", "nyaan.hpp"]:
    shutil.copyfile(source / name, work / name)
scripts = {
    "candidates": [("benchmark-third.cpp.txt", "benchmark.cpp"), ("run-third.py.txt", "run.py")],
    "final": [("final-benchmark.cpp", "final-benchmark.cpp"), ("run-final.py", "run-final.py")],
    "validate": [("benchmark.cpp", "benchmark.cpp"), ("verify.cpp", "verify.cpp"), ("run-verify.py", "run-verify.py")],
    "official": [("official.cpp", "official.cpp"), ("official-nyaan.cpp", "official-nyaan.cpp"), ("run-official.py", "run-official.py")],
}
for src, dst in scripts[args.mode]:
    shutil.copyfile(source / src, work / dst)
if args.mode == "official":
    runner = work / "run-official.py"
    text = runner.read_text(encoding="utf-8").replace("here = root / '.build/kth-root-research'", "here = Path(__file__).resolve().parent")
    runner.write_text(text, encoding="utf-8", newline="\n")
entry = {"candidates":"run.py", "final":"run-final.py", "validate":"run-verify.py", "official":"run-official.py"}[args.mode]
print(work, flush=True)
raise SystemExit(subprocess.call([sys.executable, str(work / entry)], cwd=root))
