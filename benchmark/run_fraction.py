#!/usr/bin/env python3
"""Same-input rational arithmetic comparison; retains errors and raw measurements."""
import hashlib
import json
from pathlib import Path
import platform
import statistics
import subprocess
from datetime import datetime, timezone

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "benchmark/results/legacy-fraction"
OUTPUT.mkdir(parents=True, exist_ok=True)
BINARY = ROOT / ".build/legacy-fraction/benchmark"
BINARY.parent.mkdir(parents=True, exist_ok=True)
FLAGS = ["-std=gnu++20", "-O2", "-DNDEBUG", "-Wall", "-Wextra", "-Werror", "-I."]
before_header = ROOT / ".build/legacy-fraction/before.hpp"
if before_header.exists():
    before = before_header.read_text()
else:
    before = json.loads((OUTPUT / "results.json").read_text())["before_source"]
generated_header = ROOT / ".build/legacy-fraction/before-namespaced.hpp"
generated_header.write_text(before.replace("namespace blueberry", "namespace blueberry_before"))
variant_flag = f'-DBLUEBERRY_FRACTION_BEFORE_HEADER="{generated_header}"'
compiled = subprocess.run(["g++", *FLAGS, variant_flag, "benchmark/fraction.cpp", "-o", str(BINARY)],
                          cwd=ROOT, capture_output=True, text=True)
(OUTPUT / "compile.log").write_text(compiled.stdout + compiled.stderr)
compiled.check_returncode()
rows = []
for operation in ("construct-compare", "arithmetic"):
    for repeat in range(-1, 5):
        for variant in ("before", "blueberry", "boost"):
            result = subprocess.run([str(BINARY), variant, operation], cwd=ROOT,
                                    capture_output=True, text=True)
            row = dict(operation=operation, repeat=repeat, variant=variant,
                       returncode=result.returncode, stdout=result.stdout, stderr=result.stderr)
            rows.append(row)
            (OUTPUT / "raw.json").write_text(json.dumps(rows, indent=2) + "\n")
            result.check_returncode()
            ns, checksum = map(int, result.stdout.split())
            row.update(nanoseconds=ns, checksum=checksum)
for operation in ("construct-compare", "arithmetic"):
    assert len({r["checksum"] for r in rows if r["operation"] == operation}) == 1
summary = []
for operation in ("construct-compare", "arithmetic"):
    for variant in ("before", "blueberry", "boost"):
        samples = [r["nanoseconds"] for r in rows if r["operation"] == operation
                   and r["variant"] == variant and r["repeat"] >= 0]
        summary.append(dict(operation=operation, variant=variant, median_ns=statistics.median(samples),
                            min_ns=min(samples), samples_ns=samples))
paths = [ROOT / "benchmark/fraction.cpp", ROOT / "blueberry/math/fraction.hpp",
         Path("/usr/include/boost/rational.hpp"), Path("/usr/include/boost/version.hpp")]
report = dict(generated_at=datetime.now(timezone.utc).isoformat(),
              revision=subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
              compiler=subprocess.check_output(["g++", "--version"], text=True).splitlines()[0],
              flags=FLAGS, generated_variant_flag=variant_flag, platform=platform.platform(),
              before_source=before, before_sha256=hashlib.sha256(before.encode()).hexdigest(),
              cpu=subprocess.check_output(["lscpu"], text=True),
              seed=20260917, size=300000, warmups=1, repeats=5,
              sha256={str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
              summary=summary,
              note="Bounds deliberately keep both implementations in their contracts. Input generation excluded; constructors included. Unbounded cpp_int oracle testing is separate.")
(OUTPUT / "raw.json").write_text(json.dumps(rows, indent=2) + "\n")
(OUTPUT / "results.json").write_text(json.dumps(report, indent=2) + "\n")
print(json.dumps(summary, indent=2))
