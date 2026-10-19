#!/usr/bin/env python3
"""Keep GCC's generated instructions for the one-line update-loop comparison."""
import json
import subprocess
from pathlib import Path
import run as common


def main():
    output = common.HERE / "update-disassembly"
    if output.exists():
        raise RuntimeError("refusing to overwrite existing disassembly")
    output.mkdir()
    source = common.HERE / "update-kernel.cpp"
    metadata = {"compiler": subprocess.check_output(["g++", "--version"], text=True),
                "source_sha256": common.sha256(source), "commands": []}
    for name, filename in [("fixed-four", "control-segment-tree.hpp.snapshot"),
                           ("skipped-vectors", "segment-tree.hpp.snapshot")]:
        header = common.HERE / "skipped-vector-candidate" / filename
        command = ["g++", "-std=gnu++20", "-O2", "-DNDEBUG",
                   f'-DBLUEBERRY_EXTRA_FAST_HEADER="{header}"',
                   "-S", str(source), "-o", str(output / (name + ".s"))]
        result = subprocess.run(command, capture_output=True, text=True)
        metadata["commands"].append({"argv": command, "header_sha256": common.sha256(header),
                                     "returncode": result.returncode, "stdout": result.stdout,
                                     "stderr": result.stderr})
        common.write_json(output / "metadata.json", metadata)
        result.check_returncode()
    print("Saved fixed/skipped GCC O2 instruction listings", flush=True)


if __name__ == "__main__":
    main()
