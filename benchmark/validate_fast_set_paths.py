#!/usr/bin/env python3
import hashlib
import json
from pathlib import Path
import platform
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / ".build/fast-set-validation"
OUTPUT = ROOT / "benchmark/results/fast-set-validation-guarded.json"
SOURCE = ROOT / "tests/random/fast-set-initialization.cpp"
HEADER = ROOT / "blueberry/data-structure/fast-set.hpp"


def main():
    BUILD.mkdir(parents=True, exist_ok=True)
    avx2 = any(line.startswith("flags") and " avx2 " in f" {line.split(':', 1)[1]} "
               for line in Path("/proc/cpuinfo").read_text().splitlines())
    result = {"timestamp_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
              "platform": platform.platform(), "header_sha256": hashlib.sha256(HEADER.read_bytes()).hexdigest(),
              "test_sha256": hashlib.sha256(SOURCE.read_bytes()).hexdigest(), "avx2_available": avx2,
              "cases": [], "status": "not_completed"}
    configs = {"runtime": [], "sse2": ["-D__builtin_cpu_supports(x)=0"],
               "scalar": ["-D__builtin_cpu_supports(x)=0", "-U__SSE2__"],
               "no_popcnt": ["-D__builtin_cpu_supports(x)=((x)[0]=='a')"],
               "no_avx2": ["-D__builtin_cpu_supports(x)=((x)[0]=='p')"],
               "compile_avx2": ["-mavx2"]}
    try:
        for compiler in ("g++", "clang++"):
            version = subprocess.check_output([compiler, "--version"], text=True).splitlines()[0]
            for standard in ("gnu++20", "gnu++23"):
                for name, options in configs.items():
                    row = {"compiler": version, "standard": standard, "mode": name, "status": "not_run"}
                    result["cases"].append(row)
                    if name == "compile_avx2" and not avx2:
                        row["status"] = "skipped_no_avx2"
                        continue
                    exe = BUILD / f"{compiler}-{standard}-{name}"
                    command = [compiler, f"-std={standard}", "-O2", "-DNDEBUG", "-Wall", "-Wextra", *options,
                               "-I", str(ROOT), str(SOURCE), "-o", str(exe)]
                    row["command"] = command
                    compiled = subprocess.run(command, text=True, capture_output=True)
                    row["compile_exit"] = compiled.returncode
                    row["compile_stdout"] = compiled.stdout
                    row["compile_stderr"] = compiled.stderr
                    if compiled.returncode:
                        row["status"] = "compile_failed"
                        raise RuntimeError(f"compile failed: {compiler} {standard} {name}")
                    row["runs"] = []
                    for seed in (1, 2, 17):
                        run = subprocess.run([str(exe), str(seed)], text=True, capture_output=True)
                        row["runs"].append({"seed": seed, "exit": run.returncode,
                                            "stdout": run.stdout, "stderr": run.stderr})
                        if run.returncode:
                            row["status"] = "run_failed"
                            raise RuntimeError(f"run failed: {compiler} {standard} {name} seed={seed}")
                    row["status"] = "passed"
                    print(compiler, standard, name, "passed", flush=True)
        result["status"] = "completed"
    except Exception as error:
        result["error"] = str(error)
        raise
    finally:
        OUTPUT.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
