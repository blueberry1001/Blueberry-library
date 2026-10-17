#!/usr/bin/env python3
"""Fetch immutable comparison sources and build the sequence harness."""
import hashlib
import json
from pathlib import Path
import subprocess
from urllib.request import urlopen

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
BUILD = ROOT / ".build/fps-sequences"
SUBMISSIONS = {17797, 65329, 406022, 207699}


def main():
    BUILD.mkdir(parents=True, exist_ok=True)
    for record in json.loads((HERE / "source-hashes.json").read_text()):
        relative = Path(record["path"].replace("\\", "/"))
        if not relative.name.startswith("submission-"):
            continue
        identifier = int(relative.stem.removeprefix("submission-"))
        if identifier not in SUBMISSIONS:
            continue
        destination = ROOT / relative
        assert destination.resolve().is_relative_to(BUILD.resolve())
        if not destination.exists():
            with urlopen(f"https://v3.api.judge.yosupo.jp/submissions/{identifier}", timeout=30) as response:
                source = json.load(response)["source"].encode("utf-8")
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes(source)
        if hashlib.sha256(destination.read_bytes()).hexdigest() != record["sha256"]:
            raise RuntimeError(f"source hash mismatch: submission {identifier}")
        print(f"Verified submission {identifier}", flush=True)
    subprocess.run(["g++", "-std=gnu++20", "-O2", "-DNDEBUG", "-I.",
                    "-isystem", ".deps/ac-library", str(HERE / "compare.cpp"),
                    "-o", str(BUILD / "compare")], cwd=ROOT, check=True)


if __name__ == "__main__":
    main()
