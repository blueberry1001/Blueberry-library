"""Focused WSL validation, independent of catalog updates owned by integration."""
from pathlib import Path
import json
import os
import platform
import re
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "benchmark/results/contest-template-import"
BUILD = ROOT / ".build/contest-import"
ACL = Path(os.environ.get("ACL_ROOT", "/home/blueberry/lib/acl"))
HEADERS = ["blueberry/utility/contest-helpers.hpp", "blueberry/utility/contest-template.hpp", "blueberry/math/combinations.hpp"]
OUT.mkdir(parents=True, exist_ok=True)
BUILD.mkdir(parents=True, exist_ok=True)
cpu = next((line.split(":", 1)[1].strip() for line in Path("/proc/cpuinfo").read_text().splitlines()
            if line.startswith("model name")), "unknown")
(OUT/"environment.json").write_text(json.dumps(dict(os=platform.platform(), cpu=cpu,
    flags="-std=gnu++20 -O2 -DNDEBUG", acl_path=str(ACL),
    benchmark_n=1000000, benchmark_q=1000000, compression_n=200000,
    runs=5, seed=12345, timing="steady_clock; construction/allocation and query loops separate; I/O outside timed regions",
    memory="original three int64 tables vs adopted two ACL Mint int32 tables; vector capacity overhead not included"), indent=2)+"\n")
results = []

def run(command, label, input_text=None):
    start = time.monotonic()
    result = subprocess.run(command, cwd=ROOT, text=True, input=input_text, capture_output=True)
    row = dict(label=label, command=list(map(str, command)), returncode=result.returncode,
               seconds=time.monotonic()-start, stdout=result.stdout, stderr=result.stderr)
    results.append(row)
    (OUT / "validation.json").write_text(json.dumps(results, ensure_ascii=False, indent=2)+"\n")
    if result.returncode:
        raise RuntimeError(f"{label}: {result.stderr}")
    return result.stdout

for compiler in ["g++", "clang++"]:
    run([compiler, "--version"], f"{compiler} version")
    for standard in ["gnu++20", "gnu++23"]:
        flags = [compiler, f"-std={standard}", "-O2", "-Wall", "-Wextra", "-Wshadow", "-Werror", "-I.", "-isystem", str(ACL)]
        for header in HEADERS:
            run([*flags, "-x", "c++", "-", "-o", str(BUILD/"include")], f"include {compiler} {standard} {header}",
                f'#include "{header}"\n#include "{header}"\nint main() {{}}\n')
        program = BUILD/f"random-{compiler}-{standard}"
        run([*flags, "tests/random/contest-helpers.cpp", "-o", str(program)], f"random compile {compiler} {standard}")
        for seed in range(1, 6): run([str(program), str(seed)], f"random {compiler} {standard} seed={seed}")
        verify = BUILD/f"verify-{compiler}-{standard}"
        run([*flags, "verify/math/binomial-coefficient-prime-mod.test.cpp", "-o", str(verify)], f"verify compile {compiler} {standard}")
        output = run([str(verify)], f"verify smoke {compiler} {standard}", "4 7\n0 0\n5 2\n6 3\n2 3\n")
        assert output == "1\n3\n6\n0\n"
        stream_verify = BUILD/f"stream-verify-{compiler}-{standard}"
        run([*flags, "verify/utility/contest-template-128bit.test.cpp", "-o", str(stream_verify)], f"stream verify compile {compiler} {standard}")
        output = run([str(stream_verify)], f"stream verify smoke {compiler} {standard}", "2\n-2 3\n170141183460469231731687303715884105727 0\n")
        assert output == "1\n170141183460469231731687303715884105727\n"
        for header in HEADERS:
            document = ROOT/"docs"/Path(header.removeprefix("blueberry/")).with_suffix(".md")
            text = document.read_text()
            assert f"documentation_of: //{header}" in text
            assert "<!-- library-implementation -->" in text
            blocks = re.findall(r"```cpp\n(.*?)\n```", text, re.S)
            protected = re.findall(r"{% raw %}\s*```cpp\n(.*?)\n```\s*{% endraw %}", text, re.S)
            assert len(blocks) == len(protected)
            for _, body in re.findall(r'<details class="api-operation" id="([^"]+)" markdown="1">(.*?)</details>', text, re.S):
                assert "<summary>" in body and "注意点:" in body and "```cpp" in body
            run([*flags, "-x", "c++", "-", "-o", str(BUILD/"example")], f"docs compile {compiler} {standard} {header}", blocks[0])
            run([str(BUILD/"example")], f"docs run {compiler} {standard} {header}")
        # ACL itself targets one TU and has non-inline definitions. Verify our
        # independent headers here; opt-in contest-template preserves ACL's scope.
        (BUILD/"field.hpp").write_text('#pragma once\nstruct Field { int x; Field(int v=0):x((v%7+7)%7){} Field operator*(Field b)const{return Field(x*b.x);} Field inv()const{for(int v=1;v<7;++v)if(x*v%7==1)return Field(v); std::abort();} };\n')
        includes = '#include <cstdlib>\n#include "blueberry/utility/contest-helpers.hpp"\n#include "blueberry/math/combinations.hpp"\n#include "field.hpp"\n'
        (BUILD/"a.cpp").write_text(includes+'int other() { blueberry::Combinations<Field> c(6); return c.C(5,2).x; }\n')
        (BUILD/"b.cpp").write_text(includes+'int other(); int main() { blueberry::Combinations<Field> c(6); assert(other()==3 && c.P(5,2).x==6); assert(blueberry::range_sum(1,4)==10); }\n')
        run([*flags, str(BUILD/"a.cpp"), str(BUILD/"b.cpp"), "-o", str(BUILD/"odr")], f"two-TU ODR {compiler} {standard}")
        run([str(BUILD/"odr")], f"two-TU run {compiler} {standard}")
    binary = BUILD/f"bench-{compiler}"
    run([compiler, "-std=gnu++20", "-O2", "-DNDEBUG", "-I.", "-isystem", str(ACL), "benchmark/contest-template-import.cpp", "-o", str(binary)], f"benchmark compile {compiler}")
    csv = run([str(binary), "998244353"], f"benchmark run {compiler}")
    (OUT/f"{compiler}.csv").write_text(csv)
print(f"Passed {len(results)} focused checks; logs: {OUT}")
