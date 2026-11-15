#!/usr/bin/env python3
import gzip
import hashlib
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / ".build/fast-set-codegen"
OUTPUT = ROOT / "benchmark/results/fast-set-codegen-guarded"
HEADERS = {"baseline": ROOT / "benchmark/fast-set-assembly-baseline.hpp.txt",
           "candidate": ROOT / "blueberry/data-structure/fast-set.hpp"}
DRIVER = ROOT / "benchmark/fast-set-assembly-codegen.cpp"
FLAGS = ["-std=gnu++20", "-O2", "-DNDEBUG"]


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    BUILD.mkdir(parents=True, exist_ok=True)
    OUTPUT.mkdir(parents=True, exist_ok=True)
    records = []
    for compiler, label in (("g++", "gcc"), ("clang++", "clang")):
        version = subprocess.check_output([compiler, "--version"], text=True).splitlines()[0]
        for name, header in HEADERS.items():
            include = BUILD / name
            include.mkdir(exist_ok=True)
            source = header.read_bytes()
            (include / "fast-set-assembly.hpp").write_bytes(source)
            asm = BUILD / f"{label}-{name}.s"
            command = [compiler, *FLAGS, "-I", str(include), "-S", str(DRIVER), "-o", str(asm)]
            subprocess.run(command, check=True)
            contents = asm.read_bytes()
            path = OUTPUT / f"{label}-{name}.s.gz"
            with gzip.open(path, "wb") as stream:
                stream.write(contents)
            lines = contents.decode().splitlines()
            matches = [{"line": i + 1, "text": line.strip()} for i, line in enumerate(lines)
                       if re.search(r"\b(v?pcmpeqb|v?pmovmskb|popcntq?|__popcountdi2|tzcntq?|bsfq?)\b", line)]
            records.append({"compiler": version, "flags": FLAGS, "variant": name, "header_sha256": sha(source),
                            "driver_sha256": sha(DRIVER.read_bytes()), "assembly_sha256": sha(contents),
                            "assembly": path.name, "matches": matches, "command": command})
    (OUTPUT / "summary.json").write_text(json.dumps(records, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(records, indent=2))


if __name__ == "__main__":
    main()
